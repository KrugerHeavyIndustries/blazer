// vim:set et ts=3 sw=3:
// __  __ ______ _______ _______ _______ ______ 
// |  |/  |   __ \   |   |     __|    ___|   __ \
// |     <|      <   |   |    |  |    ___|      <
// |__|\__|___|__|_______|_______|_______|___|__|
//        H E A V Y  I N D U S T R I E S
//
// Copyright (C) 2016 Kruger Heavy Industries
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
   
#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include "multidict.h"

struct CommandLine {

    MultiDict opts;
    std::vector<std::string> words;
    
    // flags that have parameter values
    // -X value or -Xvalue, where -x is a flag from this set.
    // Flags that have parameter values must always take that value, defaults are not
    // supported.
    std::set<std::string> flagParams;
    
    bool flagSet(const std::string & flag) const { return opts.exists(flag); }
    
    void parse(int argc, char* argv[]) {
        int j = 0;
        while (j < argc) {
            if (argv[j][0] == '-') {
                std::string flag = std::string(argv[j], 0, 2), value;
                if (flagParams.find(flag) != flagParams.end()) {
                    // is flag with parameter
                    if (strlen(argv[j]) == 2 && argc >= j+1)
                        opts.insert(flag, argv[++j]);// value is next string, insert and skip
                    else// value is part of string, cut out
                        opts.insert(flag, std::string(argv[j], 2, strlen(argv[j]) - 2));
                } else {
                    // is plain boolean flag
                    opts.insert(flag, "");
                }
            } else {
                words.push_back(argv[j]);
            }
            ++j;
        }
    }
};
#endif // COMMANDLINE_H
