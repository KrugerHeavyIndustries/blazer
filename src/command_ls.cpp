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

#include "command_ls.h"

#include <ostream>
#include <algorithm>
#include <functional>

#include "bb.h"
#include "commandline.h"

namespace khi { 
namespace command { 

using std::string; 
using std::list;

bool Ls::valid(size_t wordc) {
   return (wordc == 2 || wordc == 3 || wordc == 4);
}

int Ls::execute(size_t wordc, CommandLine& cmds, BB& bb) { 
   listBucket(cmds, bb);
   return EXIT_SUCCESS;
}

void Ls::printUsage() { 
    std::cout << "List bucket contents:" << std::endl;
    std::cout << "\tblazer ls <bucketName> [<startFileName>] [<maxFileCount>]" << std::endl;
    std::cout << std::endl;
}

void Ls::listBucket(CommandLine& cmds, BB& bb) {
   int idx = 1; 
   string bucketName;
   string startFileName;
   string maxFileCount;

   parse3(idx, cmds, bucketName, startFileName, maxFileCount);

   list<BB_Object> files = bb.listBucket(bucketName, startFileName, parseOrDefault(maxFileCount, 100));
   std::for_each(files.begin(), files.end(), PrintObject(*this)); 
}

} // namespace command
} // namespace khi
