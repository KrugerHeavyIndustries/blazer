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

#ifndef BLAZER_IO_H
#define BLAZER_IO_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sstream>

#include "multidict.h"

namespace RestClient { 
   class Connection;
}

struct BB_Object { 
   std::string id;
   std::string name;
   int contentLength; 
   std::string contentType; 
   std::string contentSha1; 
   std::string action; 
   uint64_t uploadTimestamp;

   BB_Object() {}
};

struct BB_Bucket { 
   std::string id; 
   std::string name; 
   std::string type;
   std::list<BB_Object> objects;

   BB_Bucket(const std::string& _id, const std::string& _name, const std::string& _type) 
      : id(_id), name(_name), type(_type) {} 
};

class BB {

   std::string m_accountId;
   std::string m_applicationKey;
   std::string m_apiUrl;
   std::string m_downloadUrl;
   std::string m_authorizationToken;

   RestClient::Connection* m_connection;

   int verbosity;
   std::list<BB_Bucket> buckets;

   static const std::string API_URL_PATH; 
    
   static void parseBucketsList(std::list<BB_Bucket>& buckets, const std::string& json);
   static void parseObjectsList(std::list<BB_Object>& objects, const std::string& xml);
   
   struct UploadUrlInfo { 
      std::string bucketId; 
      std::string uploadUrl; 
      std::string authorizationToken;
   };

   bool getUploadUrl(UploadUrlInfo& info); 
    
  public:

   BB(const std::string& accountId, const std::string& applicationKey);
   ~BB();

   void authorize();

   void setupConnection(const std::string& baseUrl);
    
   void setVerbosity(int v) { verbosity = v; }
    
   std::list<BB_Bucket>& getBuckets(bool getContents, bool refresh);
                                     
   void refreshBuckets(bool getContents);
    
   void getBucketContents(BB_Bucket& bucket);
    
   void uploadFile(const std::string& bucket, const std::string& name, const std::string& contentType);
   
   void downloadFileById(const std::string& key, std::ofstream& fout); 
     
   void downloadFileByName(const std::string& bucket, const std::string& name, std::ofstream& fout); 
 
   std::string listBuckets();
    
   std::string listBucket(const std::string& bkt);
    
};
#endif // BLAZER_IO_H 
