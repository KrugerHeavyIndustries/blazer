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

#include "bb.h"
#include "commandline.h"

namespace khi { 
namespace command { 

bool Ls::valid(size_t wordc) {
   return (wordc == 1 || wordc == 2 || wordc == 3);
}

int Ls::execute(size_t wordc, CommandLine& cmds, BB& bb) { 
   switch (wordc) {
   case 1:
      listBuckets(cmds, bb);
      return EXIT_SUCCESS;
   case 2:
   case 3:
      return EXIT_SUCCESS;
   }
   return EXIT_FAILURE;
}

void Ls::printUsage() { 
    std::cout << "List all buckets:" << std::endl;
    std::cout << "\tblazer ls" << std::endl;
    std::cout << "List all buckets with contents:" << std::endl;
    std::cout << "\tblazer -r ls" << std::endl;
    std::cout << "List contents of bucket or object from bucket:" << std::endl;
    std::cout << "\tblazer ls BUCKET_NAME [OBJECT_KEY]" << std::endl;
    std::cout << std::endl;
}

void Ls::listBuckets(CommandLine& cmds, BB& bb) { 
   const bool recurse = cmds.hasFlag("-r");
   std::list<BB_Bucket>& buckets = bb.getBuckets(recurse, true);
   std::list<BB_Bucket>::iterator bkt;

   std::cout << "Buckets:" << std::endl;
   for (bkt = buckets.begin(); bkt != buckets.end(); ++bkt) {
      if (recurse) {
         printBucket(*bkt, true);
      } else {
         std::cout << bkt->name << " (" << bkt->id << ")" << std::endl;
      }
   }
}

} // namespace command
} // namespace khi
