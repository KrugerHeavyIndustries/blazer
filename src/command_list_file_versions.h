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

#ifndef COMMAND_LIST_FILE_VERSIONS_H
#define COMMAND_LIST_FILE_VERSIONS_H

#include "command.h"

#include <list> 

namespace khi { 
namespace command { 

struct ListFileVersions : Base { 

   bool valid(size_t wordc);

   int execute(size_t wordc, CommandLine& cmds, BB& bb);

   void printUsage();

   private: 

   const std::list<BB_Object> select(size_t wordc, CommandLine& cmds, BB& bb);

   int listFiles(const std::list<BB_Object>& objects);

   int maxFileCount(const std::string& str);
};

} // namespace khi
} // namespace commands

#endif // COMMAND_LIST_FILE_VERSIONS_H
