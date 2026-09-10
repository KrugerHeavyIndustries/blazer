// vim:set et ts=3 sw=3:
// __  __ ______ _______ _______ _______ ______
// |  |/  |   __ \   |   |     __|    ___|   __ \
// |     <|      <   |   |    |  |    ___|      <
// |__|\__|___|__|_______|_______|_______|___|__|
//        H E A V Y  I N D U S T R I E S
//
// Copyright (C) 2024 Kruger Heavy Industries
// http://www.krugerheavyindustries.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef _KHI_CLI_H_
#define _KHI_CLI_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>

namespace khi {
namespace argy {

enum class ArgumentKind {
    POSITIONAL,
    OPTION,
    FLAG
};

struct ArgumentDefinition {
    std::string name;
    std::string long_name;
    std::string short_name;
    std::string help;
    ArgumentKind kind;
    bool required;
    std::string default_value;
    bool is_array;
    bool hidden;
};

inline std::string to_kebab_case(const std::string& camel) {
    std::string result;
    for (size_t i = 0; i < camel.size(); ++i) {
        char c = camel[i];
        if (std::isupper(c)) {
            if (i > 0) result += '-';
            result += static_cast<char>(std::tolower(c));
        } else if (c == '_') {
            result += '-';
        } else {
            result += c;
        }
    }
    return result;
}

class Command;

class ArgumentSet {
public:
    void add(const ArgumentDefinition& def) {
        if (def.kind == ArgumentKind::POSITIONAL) {
            ++m_positional_count;
        }
        m_defs.push_back(def);
    }

    const std::vector<ArgumentDefinition>& definitions() const {
        return m_defs;
    }

    const ArgumentDefinition* find_by_long(const std::string& name) const {
        for (const auto& d : m_defs) {
            if (d.long_name == name) return &d;
        }
        return nullptr;
    }

    const ArgumentDefinition* find_by_short(const std::string& name) const {
        for (const auto& d : m_defs) {
            if (d.short_name == name) return &d;
        }
        return nullptr;
    }

    const ArgumentDefinition* find_positional(size_t index) const {
        size_t pos = 0;
        for (const auto& d : m_defs) {
            if (d.kind == ArgumentKind::POSITIONAL) {
                if (pos == index) return &d;
                ++pos;
            }
        }
        return nullptr;
    }

private:
    std::vector<ArgumentDefinition> m_defs;
    size_t m_positional_count = 0;
};

class ParsedValues {
public:
    void set(const std::string& name, const std::string& value) {
        m_values[name] = {value};
    }

    void append(const std::string& name, const std::string& value) {
        m_values[name].push_back(value);
    }

    void set_flag(const std::string& name, bool value) {
        m_flags[name] = value;
    }

    std::optional<std::string> get(const std::string& name) const {
        auto it = m_values.find(name);
        if (it != m_values.end() && !it->second.empty()) {
            return it->second.back();
        }
        return std::nullopt;
    }

    std::vector<std::string> get_all(const std::string& name) const {
        auto it = m_values.find(name);
        if (it != m_values.end()) {
            return it->second;
        }
        return {};
    }

    bool get_flag(const std::string& name) const {
        auto it = m_flags.find(name);
        if (it != m_flags.end()) {
            return it->second;
        }
        return false;
    }

    bool has(const std::string& name) const {
        auto vit = m_values.find(name);
        if (vit != m_values.end() && !vit->second.empty()) return true;
        auto fit = m_flags.find(name);
        if (fit != m_flags.end()) return true;
        return false;
    }

private:
    std::map<std::string, std::vector<std::string>> m_values;
    std::map<std::string, bool> m_flags;
};

struct CommandConfiguration {
    std::string name;
    std::string abstract;
    std::string discussion;
    std::string version;
};

class ArgumentSlot {
public:
    virtual ~ArgumentSlot() = default;
    virtual ArgumentDefinition definition() const = 0;
    virtual void populate(const ParsedValues& values) = 0;
};

class Command {
public:
    virtual ~Command() = default;

    virtual CommandConfiguration configuration() const = 0;
    virtual void define_arguments(ArgumentSet& args) const;
    virtual void populate(const ParsedValues& values);
    virtual std::optional<std::string> validate() const;
    virtual int run() = 0;

    void add_subcommand(std::unique_ptr<Command> sub) {
        sub->set_parent(this);
        m_subcommands.push_back(std::move(sub));
    }

    const std::vector<std::unique_ptr<Command>>& subcommands() const {
        return m_subcommands;
    }

    Command* find_subcommand(const std::string& name) const {
        for (const auto& sub : m_subcommands) {
            if (sub->resolved_name() == name) return sub.get();
        }
        return nullptr;
    }

    const Command* parent() const { return m_parent; }
    void set_parent(Command* p) { m_parent = p; }

    void register_slot(ArgumentSlot* slot) {
        m_slots.push_back(slot);
    }

    std::string resolved_name() const {
        auto config = configuration();
        if (!config.name.empty()) return config.name;
        return "Command";
    }

private:
    std::vector<std::unique_ptr<Command>> m_subcommands;
    std::vector<ArgumentSlot*> m_slots;
    Command* m_parent = nullptr;
};

inline void Command::define_arguments(ArgumentSet& args) const {
    for (const auto* slot : m_slots) {
        args.add(slot->definition());
    }
}

inline void Command::populate(const ParsedValues& values) {
    for (auto* slot : m_slots) {
        slot->populate(values);
    }
}

inline std::optional<std::string> Command::validate() const {
    return std::nullopt;
}

class HelpGenerator {
public:
    static std::string generate(const Command& cmd) {
        auto config = cmd.configuration();
        std::string out;

        if (!config.abstract.empty()) {
            out += fmt::format("{}\n\n", config.abstract);
        }

        out += fmt::format("Usage: {}\n", usage_line(cmd));

        ArgumentSet args;
        cmd.define_arguments(args);

        std::vector<const ArgumentDefinition*> positionals;
        std::vector<const ArgumentDefinition*> options;
        std::vector<const ArgumentDefinition*> flags;

        for (const auto& def : args.definitions()) {
            if (def.hidden) continue;
            switch (def.kind) {
                case ArgumentKind::POSITIONAL:
                    positionals.push_back(&def);
                    break;
                case ArgumentKind::OPTION:
                    options.push_back(&def);
                    break;
                case ArgumentKind::FLAG:
                    flags.push_back(&def);
                    break;
            }
        }

        if (!positionals.empty()) {
            out += "\nArguments:\n";
            for (const auto* def : positionals) {
                std::string label = def->is_array
                    ? fmt::format("  <{}>...", def->name)
                    : fmt::format("  <{}>", def->name);
                out += format_column(label, def->help);
            }
        }

        if (!options.empty() || !flags.empty()) {
            out += "\nOptions:\n";
            for (const auto* def : options) {
                std::string suffix = def->is_array ? "..." : "";
                std::string label;
                if (!def->short_name.empty()) {
                    label = fmt::format("  -{}, --{} <{}>{}", def->short_name, def->long_name, def->name, suffix);
                } else {
                    label = fmt::format("  --{} <{}>{}", def->long_name, def->name, suffix);
                }
                out += format_column(label, def->help);
            }
            for (const auto* def : flags) {
                std::string label;
                if (!def->short_name.empty()) {
                    label = fmt::format("  -{}, --{}", def->short_name, def->long_name);
                } else {
                    label = fmt::format("  --{}", def->long_name);
                }
                out += format_column(label, def->help);
            }
            out += format_column("  -h, --help", "Show help information.");
        }

        const auto& subs = cmd.subcommands();
        if (!subs.empty()) {
            out += "\nCommands:\n";
            for (const auto& sub : subs) {
                auto sub_config = sub->configuration();
                out += format_column(fmt::format("  {}", sub->resolved_name()), sub_config.abstract);
            }
        }

        if (!config.discussion.empty()) {
            out += fmt::format("\n{}\n", config.discussion);
        }

        return out;
    }

    static std::string usage_line(const Command& cmd) {
        std::string out = cmd.resolved_name();

        ArgumentSet args;
        cmd.define_arguments(args);

        for (const auto& def : args.definitions()) {
            switch (def.kind) {
                case ArgumentKind::POSITIONAL:
                    if (def.required) {
                        out += def.is_array
                            ? fmt::format(" <{}>...", def.name)
                            : fmt::format(" <{}>", def.name);
                    } else {
                        out += def.is_array
                            ? fmt::format(" [<{}>...]", def.name)
                            : fmt::format(" [<{}>]", def.name);
                    }
                    break;
                case ArgumentKind::OPTION:
                    if (def.required) {
                        out += fmt::format(" --{} <{}>", def.long_name, def.name);
                    } else {
                        out += fmt::format(" [--{} <{}>]", def.long_name, def.name);
                    }
                    break;
                case ArgumentKind::FLAG:
                    out += fmt::format(" [--{}]", def.long_name);
                    break;
            }
        }

        if (!cmd.subcommands().empty()) {
            out += " <command>";
        }

        return out;
    }

private:
    static constexpr int m_column_width = 24;

    static std::string format_column(const std::string& label, const std::string& description) {
        if (static_cast<int>(label.size()) < m_column_width) {
            return fmt::format("{}{}{}\n", label, std::string(m_column_width - label.size(), ' '), description);
        }
        return fmt::format("{}\n{}{}\n", label, std::string(m_column_width, ' '), description);
    }
};

template<typename T>
struct parse_value;

template<>
struct parse_value<std::string> {
    static std::string from(const std::string& s) { return s; }
    static std::string to_default(const std::string& v) { return v; }
};

template<>
struct parse_value<int> {
    static int from(const std::string& s) { return std::stoi(s); }
    static std::string to_default(int v) { return std::to_string(v); }
};

template<>
struct parse_value<long> {
    static long from(const std::string& s) { return std::stol(s); }
    static std::string to_default(long v) { return std::to_string(v); }
};

template<>
struct parse_value<double> {
    static double from(const std::string& s) { return std::stod(s); }
    static std::string to_default(double v) { return std::to_string(v); }
};

template<>
struct parse_value<float> {
    static float from(const std::string& s) { return std::stof(s); }
    static std::string to_default(float v) { return std::to_string(v); }
};

template<>
struct parse_value<bool> {
    static bool from(const std::string& s) {
        return s == "true" || s == "1" || s == "yes";
    }
    static std::string to_default(bool v) { return v ? "true" : "false"; }
};

template<typename T>
class Argument : public ArgumentSlot {
public:
    Argument(Command* cmd, const std::string& name, const std::string& help_text)
        : m_name(name), m_help(help_text), m_required(true) {
        cmd->register_slot(this);
    }

    ArgumentDefinition definition() const override {
        return {m_name, "", "", m_help, ArgumentKind::POSITIONAL, m_required, "", false, false};
    }

    void populate(const ParsedValues& values) override {
        if (auto v = values.get(m_name)) {
            m_value = parse_value<T>::from(*v);
        }
    }

    const T& value() const { return m_value; }
    const T& operator*() const { return m_value; }
    const T* operator->() const { return &m_value; }
    operator const T&() const { return m_value; }

private:
    std::string m_name;
    std::string m_help;
    bool m_required;
    T m_value{};
};

template<typename T>
class Argument<std::optional<T>> : public ArgumentSlot {
public:
    Argument(Command* cmd, const std::string& name, const std::string& help_text)
        : m_name(name), m_help(help_text) {
        cmd->register_slot(this);
    }

    ArgumentDefinition definition() const override {
        return {m_name, "", "", m_help, ArgumentKind::POSITIONAL, false, "", false, false};
    }

    void populate(const ParsedValues& values) override {
        if (auto v = values.get(m_name)) {
            m_value = parse_value<T>::from(*v);
        }
    }

    const std::optional<T>& value() const { return m_value; }
    const std::optional<T>& operator*() const { return m_value; }
    bool has_value() const { return m_value.has_value(); }
    operator bool() const { return m_value.has_value(); }

private:
    std::string m_name;
    std::string m_help;
    std::optional<T> m_value;
};

template<typename T>
class Argument<std::vector<T>> : public ArgumentSlot {
public:
    Argument(Command* cmd, const std::string& name, const std::string& help_text,
        bool required = true)
        : m_name(name), m_help(help_text), m_required(required) {
        cmd->register_slot(this);
    }

    ArgumentDefinition definition() const override {
        return {m_name, "", "", m_help, ArgumentKind::POSITIONAL, m_required, "", true, false};
    }

    void populate(const ParsedValues& values) override {
        for (const auto& s : values.get_all(m_name)) {
            m_value.push_back(parse_value<T>::from(s));
        }
    }

    const std::vector<T>& value() const { return m_value; }
    const std::vector<T>& operator*() const { return m_value; }
    operator const std::vector<T>&() const { return m_value; }

private:
    std::string m_name;
    std::string m_help;
    bool m_required;
    std::vector<T> m_value;
};

template<typename T>
class Option : public ArgumentSlot {
public:
    Option(Command* cmd, const std::string& long_name, const std::string& short_name,
        const std::string& help_text, T default_val = T{})
        : m_name(long_name), m_long(long_name), m_short(short_name),
          m_help(help_text), m_value(default_val), m_default(default_val) {
        cmd->register_slot(this);
    }

    ArgumentDefinition definition() const override {
        return {m_name, m_long, m_short, m_help, ArgumentKind::OPTION, false,
                parse_value<T>::to_default(m_default), false, false};
    }

    void populate(const ParsedValues& values) override {
        if (auto v = values.get(m_name)) {
            m_value = parse_value<T>::from(*v);
        }
    }

    const T& value() const { return m_value; }
    const T& operator*() const { return m_value; }
    const T* operator->() const { return &m_value; }
    operator const T&() const { return m_value; }

private:
    std::string m_name;
    std::string m_long;
    std::string m_short;
    std::string m_help;
    T m_value;
    T m_default;
};

template<typename T>
class Option<std::optional<T>> : public ArgumentSlot {
public:
    Option(Command* cmd, const std::string& long_name, const std::string& short_name,
        const std::string& help_text)
        : m_name(long_name), m_long(long_name), m_short(short_name), m_help(help_text) {
        cmd->register_slot(this);
    }

    ArgumentDefinition definition() const override {
        return {m_name, m_long, m_short, m_help, ArgumentKind::OPTION, true, "", false, false};
    }

    void populate(const ParsedValues& values) override {
        if (auto v = values.get(m_name)) {
            m_value = parse_value<T>::from(*v);
        }
    }

    const std::optional<T>& value() const { return m_value; }
    bool has_value() const { return m_value.has_value(); }
    const T& operator*() const { return *m_value; }
    const T* operator->() const { return &(*m_value); }

private:
    std::string m_name;
    std::string m_long;
    std::string m_short;
    std::string m_help;
    std::optional<T> m_value;
};

template<typename T>
class Option<std::vector<T>> : public ArgumentSlot {
public:
    Option(Command* cmd, const std::string& long_name, const std::string& short_name,
        const std::string& help_text)
        : m_name(long_name), m_long(long_name), m_short(short_name), m_help(help_text) {
        cmd->register_slot(this);
    }

    ArgumentDefinition definition() const override {
        return {m_name, m_long, m_short, m_help, ArgumentKind::OPTION, false, "", true, false};
    }

    void populate(const ParsedValues& values) override {
        for (const auto& s : values.get_all(m_name)) {
            m_value.push_back(parse_value<T>::from(s));
        }
    }

    const std::vector<T>& value() const { return m_value; }
    const std::vector<T>& operator*() const { return m_value; }
    operator const std::vector<T>&() const { return m_value; }

private:
    std::string m_name;
    std::string m_long;
    std::string m_short;
    std::string m_help;
    std::vector<T> m_value;
};

class Flag : public ArgumentSlot {
public:
    Flag(Command* cmd, const std::string& long_name, const std::string& short_name,
        const std::string& help_text)
        : m_name(long_name), m_long(long_name), m_short(short_name), m_help(help_text) {
        cmd->register_slot(this);
    }

    ArgumentDefinition definition() const override {
        return {m_name, m_long, m_short, m_help, ArgumentKind::FLAG, false, "", false, false};
    }

    void populate(const ParsedValues& values) override {
        m_value = values.get_flag(m_name);
    }

    bool value() const { return m_value; }
    bool operator*() const { return m_value; }
    operator bool() const { return m_value; }

private:
    std::string m_name;
    std::string m_long;
    std::string m_short;
    std::string m_help;
    bool m_value = false;
};

class Parser {
public:
    explicit Parser(Command& root) : m_root(&root) {}

    Parser(const std::string& name, const std::string& description,
           const std::string& version = "")
        : m_owned_root(std::make_unique<RootCommand>(name, description, version)),
          m_root(m_owned_root.get()) {}

    void add_command(std::unique_ptr<Command> cmd) {
        m_root->add_subcommand(std::move(cmd));
    }

    int parse_and_run(int argc, const char* argv[]) {
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) {
            args.emplace_back(argv[i]);
        }
        return parse_and_run(args);
    }

    int parse_and_run(const std::vector<std::string>& args) {
        auto result = parse(args);
        if (!result.ok()) {
            fmt::print(stderr, "Error: {}\n", result.error);
            fmt::print(stderr, "Usage: {}\n", HelpGenerator::usage_line(*m_root));
            fmt::print(stderr, "  Use '--help' for more information.\n");
            return 1;
        }

        apply_defaults(([&] {
            ArgumentSet a;
            result.target->define_arguments(a);
            return a;
        })(), result.values);

        ArgumentSet arg_defs;
        result.target->define_arguments(arg_defs);
        auto req_err = check_required(arg_defs, result.values);
        if (req_err) {
            fmt::print(stderr, "Error: {}\n", *req_err);
            fmt::print(stderr, "Usage: {}\n", HelpGenerator::usage_line(*m_root));
            fmt::print(stderr, "  Use '--help' for more information.\n");
            return 1;
        }

        result.target->populate(result.values);

        auto val_err = result.target->validate();
        if (val_err) {
            fmt::print(stderr, "Error: {}\n", *val_err);
            return 1;
        }

        return result.target->run();
    }

private:
    class RootCommand : public Command {
    public:
        RootCommand(const std::string& name, const std::string& description,
                     const std::string& version)
            : m_config{name, description, "", version} {}

        CommandConfiguration configuration() const override { return m_config; }
        int run() override {
            fmt::print("{}", HelpGenerator::generate(*this));
            return 0;
        }

    private:
        CommandConfiguration m_config;
    };

    std::unique_ptr<RootCommand> m_owned_root;
    Command* m_root;

    struct ParseResult {
        Command* target = nullptr;
        ParsedValues values;
        std::string error;

        bool ok() const { return error.empty(); }
    };

    ParseResult parse(const std::vector<std::string>& args) {
        Command* current = m_root;
        size_t i = 0;

        while (i < args.size() && !current->subcommands().empty()) {
            auto* sub = current->find_subcommand(args[i]);
            if (sub) {
                current = sub;
                ++i;
            } else {
                break;
            }
        }

        for (size_t j = i; j < args.size(); ++j) {
            if (args[j] == "--help" || args[j] == "-h") {
                fmt::print("{}", HelpGenerator::generate(*current));
                std::exit(0);
            }
        }

        auto config = current->configuration();
        if (!config.version.empty()) {
            for (size_t j = i; j < args.size(); ++j) {
                if (args[j] == "--version") {
                    fmt::print("{}\n", config.version);
                    std::exit(0);
                }
            }
        }

        ArgumentSet arg_defs;
        current->define_arguments(arg_defs);
        ParsedValues values;
        size_t positional_index = 0;
        bool past_double_dash = false;

        while (i < args.size()) {
            const std::string& a = args[i];

            if (a == "--" && !past_double_dash) {
                past_double_dash = true;
                ++i;
                continue;
            }

            if (!past_double_dash && a.size() > 2 && a[0] == '-' && a[1] == '-') {
                std::string name;
                std::string value;
                bool has_equals = false;

                auto eq_pos = a.find('=', 2);
                if (eq_pos != std::string::npos) {
                    name = a.substr(2, eq_pos - 2);
                    value = a.substr(eq_pos + 1);
                    has_equals = true;
                } else {
                    name = a.substr(2);
                }

                const auto* def = arg_defs.find_by_long(name);
                if (!def) {
                    return {nullptr, {}, fmt::format("unknown option '--{}'", name)};
                }

                if (def->kind == ArgumentKind::FLAG) {
                    if (has_equals) {
                        return {nullptr, {}, fmt::format("flag '--{}' does not take a value", name)};
                    }
                    values.set_flag(def->name, true);
                } else {
                    if (!has_equals) {
                        ++i;
                        if (i >= args.size()) {
                            return {nullptr, {}, fmt::format("option '--{}' requires a value", name)};
                        }
                        value = args[i];
                    }
                    if (def->is_array) {
                        values.append(def->name, value);
                    } else {
                        values.set(def->name, value);
                    }
                }
            } else if (!past_double_dash && a.size() == 2 && a[0] == '-' && a[1] != '-') {
                std::string name(1, a[1]);
                const auto* def = arg_defs.find_by_short(name);
                if (!def) {
                    return {nullptr, {}, fmt::format("unknown option '-{}'", name)};
                }

                if (def->kind == ArgumentKind::FLAG) {
                    values.set_flag(def->name, true);
                } else {
                    ++i;
                    if (i >= args.size()) {
                        return {nullptr, {}, fmt::format("option '-{}' requires a value", name)};
                    }
                    if (def->is_array) {
                        values.append(def->name, args[i]);
                    } else {
                        values.set(def->name, args[i]);
                    }
                }
            } else if (!past_double_dash && a.size() > 2 && a[0] == '-' && a[1] != '-') {
                std::string name(1, a[1]);
                const auto* def = arg_defs.find_by_short(name);
                if (!def) {
                    return {nullptr, {}, fmt::format("unknown option '-{}'", name)};
                }

                if (def->kind == ArgumentKind::FLAG) {
                    values.set_flag(def->name, true);
                    for (size_t c = 2; c < a.size(); ++c) {
                        std::string cn(1, a[c]);
                        const auto* cdef = arg_defs.find_by_short(cn);
                        if (!cdef) {
                            return {nullptr, {}, fmt::format("unknown option '-{}'", cn)};
                        }
                        if (cdef->kind != ArgumentKind::FLAG) {
                            return {nullptr, {}, fmt::format("option '-{}' requires a value and cannot be clustered", cn)};
                        }
                        values.set_flag(cdef->name, true);
                    }
                } else {
                    std::string val = a.substr(2);
                    if (def->is_array) {
                        values.append(def->name, val);
                    } else {
                        values.set(def->name, val);
                    }
                }
            } else {
                const auto* def = arg_defs.find_positional(positional_index);
                if (!def) {
                    return {nullptr, {}, fmt::format("unexpected argument '{}'", a)};
                }
                if (def->is_array) {
                    values.append(def->name, a);
                } else {
                    values.set(def->name, a);
                    ++positional_index;
                }
            }

            ++i;
        }

        return {current, std::move(values), ""};
    }

    void apply_defaults(const ArgumentSet& defs, ParsedValues& values) {
        for (const auto& def : defs.definitions()) {
            if (!values.has(def.name) && !def.default_value.empty()) {
                values.set(def.name, def.default_value);
            }
        }
    }

    std::optional<std::string> check_required(const ArgumentSet& defs, const ParsedValues& values) {
        for (const auto& def : defs.definitions()) {
            if (def.required && !values.has(def.name)) {
                if (def.kind == ArgumentKind::POSITIONAL) {
                    return fmt::format("missing required argument '<{}>'", def.name);
                } else {
                    return fmt::format("missing required option '--{}'", def.long_name);
                }
            }
        }
        return std::nullopt;
    }
};

} // namespace argy
} // namespace khi

#endif // _KHI_CLI_H_
