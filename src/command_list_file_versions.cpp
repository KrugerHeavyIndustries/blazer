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

#include "command_list_file_versions.h" 

#include <ostream>

#include "bb.h"
#include "commandline.h"

namespace khi {
namespace command {

using namespace std;

bool ListFileVersions::valid(size_t wordc) {
   return (2 <= wordc && wordc <= 5);
}

int ListFileVersions::execute(size_t wordc, CommandLine& cmds, BB& bb) { 
   return listFiles(select(wordc, cmds, bb));
}

void ListFileVersions::printUsage() { 
   cout << "Lists all the versions of all files contained in one bucket:" << endl;
   cout << "\tblazer list_file_versions <bucketName>" << endl;
   cout << endl;
}

const list<BB_Object> ListFileVersions::select(size_t wordc, CommandLine& cmds, BB& bb) { 
   switch (wordc) { 
      case 2:
         return bb.listFileVersions(cmds.words[1]);
      case 3:
         return bb.listFileVersions(cmds.words[1], cmds.words[2]);
      case 4: 
         return bb.listFileVersions(cmds.words[1], cmds.words[2], cmds.words[3]);
      case 5:
         return bb.listFileVersions(cmds.words[1], cmds.words[2], cmds.words[3], maxFileCount(cmds.words[4]));
   }
   return list<BB_Object>();
}

int ListFileVersions::listFiles(const list<BB_Object>& objects) { 
   list<BB_Object>::const_iterator iter = objects.begin();
   for ( ; iter != objects.end(); ++iter) {
      printObject(*iter, true);
   }
   return EXIT_SUCCESS; 
}

int ListFileVersions::maxFileCount(const string& str) { 
   long rv = strtol(str.c_str(), NULL, 10);
   if (rv == 0 && errno == EINVAL) { 
   }
   return static_cast<int>(rv);
}

} // namespace command 
} // namespace khi
