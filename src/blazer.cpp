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
   
#include <cmath>
#include <exception>
#include <stdexcept>
#include <memory>

#include <unistd.h>
#include <pwd.h>
#include <sys/param.h>

#include "config.h"
#include "bb.h"
#include "coding.h"
#include "mimetypes.h"
#include "multidict.h"
#include "commandline.h"
#include "command.h"
#include "exceptions.h"

#include "jsoncpp.h"

#define PATH_BLAZER_DIR ".blazer"

using namespace std;
using namespace khi;
using namespace khi::command;

void loadBlazerFile(const string& path, string& accountId, string& applicatioKey, string& name, int verbosity = 0);

void printVersion();

void printUsage(const Dispatcher& commands);

int main(int argc, char * argv[]) {
   
   int verbosity = 1;

   Dispatcher commands;

   commands.add<CreateBucket>("create_bucket");
   commands.add<DeleteBucket>("delete_bucket");
   commands.add<UpdateBucket>("update_bucket");
   commands.add<ListBuckets>("list_buckets");
   commands.add<UploadFile>("upload_file");
   commands.add<Ls>("ls");
   commands.add<FileById>("download_file_by_id");
   commands.add<FileByName>("download_file_by_name");
   commands.add<GetFileInfo>("get_file_info");
   commands.add<HideFile>("hide_file");
   commands.add<ListFileVersions>("list_file_versions");
   commands.add<DeleteFileVersion>("delete_file_version");

   MimeTypes::initialize();
    
   CommandLine cmds;
   cmds.flags.insert("-v"); // version
   cmds.flags.insert("-d"); // verbosity level
   cmds.flags.insert("-c"); // credentials file
   cmds.flags.insert("-t"); // type (Content-Type)
   cmds.flags.insert("-m"); // metadata
   cmds.parse(argc, argv);
   size_t wordc = cmds.words.size();
    
   string accountId;
   string applicationKey;
   string authorizationToken; 
   string apiUrl; 
   string downloadUrl;
   string name;
    
   struct passwd* pw = getpwuid(geteuid());
   string home(pw->pw_dir);

   if (cmds.hasFlag("-v")) {
      printVersion();
      return EXIT_SUCCESS;
   }

   if (cmds.hasFlag("-d")) {
       verbosity = cmds.opts.getWithDefault("-d", 2);
       if(verbosity > 0)
           cout << "Verbose output level " << verbosity << endl;
   }
    
   ifstream file;
        
   if (cmds.hasFlag("-c")) {
       string givenFilePath = cmds.opts.getWithDefault("-c", "");
       loadBlazerFile(givenFilePath, accountId, applicationKey, name);
   } else {
      // credentials not specified. Try standard locations
      char pwd[MAXPATHLEN];
      getcwd(pwd, MAXPATHLEN);

      string localFilePath(string(pwd) + "/" + PATH_BLAZER_DIR + "/" + "config");
      string userFilePath(home + "/" + PATH_BLAZER_DIR + "/" + "config");

      // Try ${PWD}/.blazer first
      file.open(localFilePath.c_str());
      if (file) {
          file.close();
          loadBlazerFile(localFilePath, accountId, applicationKey, name);
      } else {
          file.clear();
          file.open(userFilePath.c_str());
          if (file) {
              file.close();
              loadBlazerFile(userFilePath, accountId, applicationKey, name);
          } else {
               cerr << "Could not open blazer file" << endl;
               return EXIT_FAILURE;
          }
      }
   }

   // Remove executable name if called directly with commands, otherwise show usage
   // If symlinked, use the executable name to determine the desired operation
   // Trim to just command name
   cmds.words[0] = cmds.words[0].substr(cmds.words[0].find_last_of('/') + 1);
   if (cmds.words[0] == "blazer") {
       if (cmds.words.size() < 2) {
           printUsage(commands);
           return EXIT_SUCCESS;
       }
       cmds.words.erase(cmds.words.begin());
       --wordc;
   }
    
   // First string in cmds.words[] array is command name, following strings are parameter strings
   int result = EXIT_SUCCESS;
   if (commands.find(cmds.words[0]) != commands.end()) {
      try {
         // Create and configure blazer
         BB bb(accountId, applicationKey);
         bb.authorize();

         result = commands[cmds.words[0]]->execute(cmds.words.size(), cmds, bb);
      }
      catch (std::runtime_error& err) {
         cerr << "ERROR: " << err.what() << endl;
         return EXIT_FAILURE;
      }
      catch (ResponseError& err0) { 
         cerr << err0.what() << endl;
         return EXIT_FAILURE;
      }
   } else {
      cerr << "Did not understand command \"" << cmds.words[0] << "\"" << endl;
      return EXIT_FAILURE;
   }
   return result;
}

void loadBlazerFile(const string& path, string& accountId, string& applicationKey, string& name, int verbosity) {
    ifstream cred(path.c_str());
    if (cred) {
        while (cred) {
            string cmd;
            cred >> cmd;
            if (cmd == "accountId")
                cred >> accountId;
            else if (cmd == "applicationKey")
                cred >> applicationKey;
            else if (cmd == "name") {
                cred >> name;
            }
        }
        if (verbosity >= 2)
            cout << "using credentials from " << path << ", name: " << name << endl;
    } else {
        cerr << "Error: Could not load credentials file from " << path << "." << endl;
        exit(EXIT_FAILURE);
    }
}

void printVersion() {
   cout << "Blazer " << PACKAGE_VERSION <<  " (Backblaze B2 from the command line)" << endl;
   cout << "Copyright (c) 2016 Kruger Heavy Industries" << endl;
   cout << "http://www.krugerheavyindustries.com" << endl;
}

void printUsage(const Dispatcher& dispatcher) {
   printVersion();
   cout << endl;
   cout << "Usage:" << endl;
   dispatcher.printUsages();
}
