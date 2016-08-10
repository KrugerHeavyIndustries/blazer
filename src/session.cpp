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

#include "session.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#include <pwd.h>
#include <unistd.h>

using namespace std; 

#define PATH_BLAZER_DIR ".blazer"
#define SECONDS_IN_DAY 86400

Session::Session() { 
}

Session::Session(const string& _authorizationToken, const string& _apiUrl, const string& _downloadUrl) 
   :  authorizationToken(_authorizationToken), 
      apiUrl(_apiUrl), 
      downloadUrl(_downloadUrl)
{
}

Session Session::load() {
   Session session;
   ifstream strm(path().c_str());
   if (strm) { 
      while (strm) { 
         time_t timestamp;
         time_t currentTime = time(NULL);
         string ts; 
         strm >> ts;
         stringstream ss(ts); 
         ss >> timestamp;
         if (ss && (currentTime - timestamp) < SECONDS_IN_DAY)  {
            strm >> session.authorizationToken >> session.apiUrl >> session.downloadUrl;
         } else { 
            break; 
         }
      }
      strm.close();
   } 
   return session;
}

void Session::save() { 
   ofstream strm(path().c_str());
   if (strm && valid()) { 
      time_t ts = time(NULL);
      strm << ts << " " << authorizationToken << " " << apiUrl << " " << downloadUrl;
   }
   strm.close();
}

bool Session::valid() { 
   return !unknown();
}

bool Session::unknown() { 
   return authorizationToken.empty() && apiUrl.empty() && downloadUrl.empty();
}

string Session::path() { 
   struct passwd* pw = getpwuid(geteuid());
   return string(string(pw->pw_dir) + "/" + PATH_BLAZER_DIR + "/" + "session");
}
