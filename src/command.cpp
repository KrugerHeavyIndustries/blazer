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

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <pwd.h>
#include <sys/param.h>

#include "bb.h"

#define PATH_BLAZER_DIR ".blazer"

namespace khi {
namespace command {

using namespace std;

static void loadBlazerFile(const string& path, string& accountId, string& applicationKey) {
   ifstream cred(path.c_str());
   if (cred) {
      while (cred) {
         string cmd;
         cred >> cmd;
         if (cmd == "accountId")
            cred >> accountId;
         else if (cmd == "applicationKey")
            cred >> applicationKey;
      }
   } else {
      cerr << "Error: Could not load credentials file from " << path << "." << endl;
      exit(EXIT_FAILURE);
   }
}

unique_ptr<BB> BlazerCommand::createBB() {
   string accountId;
   string applicationKey;

   string credPath = *credentials;
   if (!credPath.empty()) {
      loadBlazerFile(credPath, accountId, applicationKey);
   } else {
      struct passwd* pw = getpwuid(geteuid());
      string home(pw->pw_dir);

      char pwd[MAXPATHLEN];
      getcwd(pwd, MAXPATHLEN);

      string localFilePath(string(pwd) + "/" + PATH_BLAZER_DIR + "/" + "config");
      string userFilePath(home + "/" + PATH_BLAZER_DIR + "/" + "config");

      ifstream file(localFilePath.c_str());
      if (file) {
         file.close();
         loadBlazerFile(localFilePath, accountId, applicationKey);
      } else {
         file.clear();
         file.open(userFilePath.c_str());
         if (file) {
            file.close();
            loadBlazerFile(userFilePath, accountId, applicationKey);
         } else {
            cerr << "Could not open blazer file" << endl;
            exit(EXIT_FAILURE);
         }
      }
   }

   auto bb = make_unique<BB>(accountId, applicationKey, test_mode);
   bb->authorize();
   return bb;
}

void BlazerCommand::printBucket(const BB_Bucket& bucket, bool bucketName) const {
   if (bucketName)
      cout << bucket.name << " (" << bucket.id << ")" << endl;
   list<BB_Object>::const_iterator obj;
   for (obj = bucket.objects.begin(); obj != bucket.objects.end(); ++obj) {
      if (bucketName) cout << "  ";
      printObject(*obj);
      cout << endl;
   }
}

void BlazerCommand::printObject(const BB_Object& object, bool longFormat) const {
   if (longFormat) {
      cout << " FileId: " << object.id << endl;
      cout << " FileName: " << object.name << endl;
      cout << " ContentLength: " << object.contentLength << endl;
      cout << " ContentType: " << object.contentType << endl;
      cout << " ContentSHA1: " << object.contentSha1 << endl;
      cout << " Action: " << object.action << endl;
      cout << " UploadTimestamp: " << object.uploadTimestamp << endl;
   } else {
      cout << object.id << " " << object.name << endl;
   }
}

} // namespace command
} // namespace khi
