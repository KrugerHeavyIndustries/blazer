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

#include "command.h" 

#include <ostream>

#include "bb.h"
#include "commandline.h"

namespace khi { 
namespace command { 

using namespace std;

void Base::printBucket(const BB_Bucket& bucket, bool bucketName) { 
   if (bucketName)
      cout << bucket.name << " (" << bucket.id << ")" << endl;
   list<BB_Object>::const_iterator obj;
   for (obj = bucket.objects.begin(); obj != bucket.objects.end(); ++obj) {
       if (bucketName) cout << "  ";
       printObject(*obj);
       cout << endl;
   }
}

void Base::printObject(const BB_Object& object, bool longFormat) { 
   if (longFormat) {
      cout << " FileId: " << object.id << endl;
      cout << " FileName: " << object.name << endl;
      cout << " ContentLength: " << object.contentLength << endl;
      cout << " ContentType: " << object.contentType << endl;
      cout << " ContentSHA1: " << object.contentSha1 << endl;
      cout << " Action: " << object.action << endl; 
      cout << " UploadTimestamp: " << object.uploadTimestamp << endl;
   } else { 
      cout << object.id;
      cout << " " << object.name;
      cout << " " << object.contentLength;
      cout << " " << object.contentType; 
      cout << " " << object.contentSha1; 
      cout << " " << object.action; 
      cout << " " << object.uploadTimestamp;
   }
}

void Base::parse1(int& idx, const CommandLine& cmds, string& position1) {
   position1 = cmds.words[idx];
   ++idx;
}

void Base::parse2(int& idx, const CommandLine& cmds, string& position1, string& position2) { 
   position1 = cmds.words[idx];
   ++idx;
   if (idx < static_cast<int>(cmds.words.size())) {
      position2 = cmds.words[idx];
      ++idx;
   } else { 
      position2 = "";
   }
}

} // namespace command
} // namespace khi
