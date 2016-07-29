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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "bb.h"
#include "coding.h"

#include "restclient-cpp/connection.h"
#include "restclient-cpp/restclient.h"

#include "jsoncpp.h"
   
using namespace std;

namespace { 
   struct find_name : std::unary_function<BB_Bucket, bool> { 
      std::string name; 
      find_name(const std::string& value) : name(value) {} 
      bool operator()(const BB_Bucket& b) const { 
         return b.name == name;
      }
   };
}

const string BB::API_URL_PATH = "/b2api/v1";

BB::BB(const string& accountId, const string& applicationKey):
   m_accountId(accountId), 
   m_applicationKey(applicationKey),
   m_connection(NULL),
   verbosity(0)
{
   RestClient::init();
}

BB::~BB() {
   RestClient::disable();
}

void BB::authorize() {
   m_connection = new RestClient::Connection("https://api.backblaze.com" + API_URL_PATH);
   m_connection->SetTimeout(20);
   m_connection->SetUserAgent("cmd/blazer");
   m_connection->SetBasicAuth(m_accountId, m_applicationKey); 

   RestClient::HeaderFields headers; 
   headers["Accept"] = "application/json";
   m_connection->SetHeaders(headers);

   RestClient::Response response = m_connection->get("/b2_authorize_account");

   khi::Json json = khi::Json::load(response.body); 
   khi::Json downloadUrl = json.get("downloadUrl");
   khi::Json apiUrl = json.get("apiUrl");
   khi::Json authorizationToken = json.get("authorizationToken"); 
   m_apiUrl = apiUrl.get<std::string>();
   m_downloadUrl = downloadUrl.get<std::string>();
   m_authorizationToken = authorizationToken.get<std::string>();

   delete m_connection; 
   m_connection = NULL;
}

void BB::setupConnection(const string& baseUrl) { 
   m_connection = new RestClient::Connection(baseUrl);
   m_connection->SetTimeout(20);
   m_connection->SetUserAgent("cmd/blazer");

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_authorizationToken;
   m_connection->SetHeaders(headers);
}

void BB::parseBucketsList(list<BB_Bucket>& buckets, const string& json) {
   khi::Json root = khi::Json::load(json); 
   if (root.isObject()) { 
      khi::Json array = root.get("buckets");
      if (array.isArray()) { 
         for (int i = 0; i < array.size(); ++i) { 
            khi::Json elem = array.at(i);
            if (elem.isObject()) { 
               khi::Json bucketId = elem.get("bucketId"); 
               khi::Json bucketName = elem.get("bucketName"); 
               khi::Json bucketType = elem.get("bucketType"); 
               buckets.push_back(BB_Bucket(bucketId.get<string>(), bucketName.get<string>(), bucketType.get<string>())); 
            }
         }
      }
   }
}

void BB::parseObjectsList(list<BB_Object>& objects, const string& json) {
   khi::Json root = khi::Json::load(json); 
   if (root.isObject()) { 
      khi::Json array = root.get("files");
      if (array.isArray()) { 
         for (int i = 0; i < array.size(); ++i) { 
            khi::Json elem = array.at(i);
            if (elem.isObject()) { 
               BB_Object object;
               khi::Json action = elem.get("action"); 
               if (action.isString())
                  object.action = action.get<string>(); 

               khi::Json contentLength = elem.get("contentLength"); 
               if (contentLength.isInteger())
                  object.contentLength = contentLength.get<int>();

               khi::Json contentType = elem.get("contentType");
               if (contentType.isString()) 
                  object.contentType = contentType.get<string>(); 

               khi::Json contentSha1 = elem.get("contentSha1");
               if (contentSha1.isString()) 
                  object.contentSha1 = contentSha1.get<string>(); 

               khi::Json fileId = elem.get("fileId"); 
               if (fileId.isString())
                  object.id = fileId.get<string>(); 

               khi::Json fileName = elem.get("fileName");
               if (fileName.isString()) 
                  object.name = fileName.get<string>();

               khi::Json uploadTimestamp = elem.get("uploadTimestamp");
               if (uploadTimestamp.isInteger()) 
                  object.uploadTimestamp = uploadTimestamp.get<int>(); 

               objects.push_back(object);
            }
         }
      }
      khi::Json nextFileName = root.get("nextFileName");
   }
}

std::list<BB_Bucket>& BB::getBuckets(bool getContents, bool refresh) {
    if (refresh || buckets.empty())
        refreshBuckets(getContents);
    return buckets;
}

void BB::refreshBuckets(bool getContents) {
    buckets.clear();
    parseBucketsList(buckets, listBuckets());
    
    if (getContents) {
        list<BB_Bucket>::iterator bkt;
        for (bkt = buckets.begin(); bkt != buckets.end(); ++bkt)
            getBucketContents(*bkt);
    }
}

void BB::getBucketContents(BB_Bucket& bucket) {
    parseObjectsList(bucket.objects, listBucket(bucket.id));
}

bool BB::getUploadUrl(UploadUrlInfo& info) { 
   bool ok = false;
   setupConnection(m_apiUrl + API_URL_PATH);
   khi::Json json = khi::Json::object();
   json.set("bucketId", khi::Json::string(info.bucketId));
   RestClient::Response response = m_connection->post("/b2_get_upload_url", json.dump());
   if (response.code == 200) {
      khi::Json json = khi::Json::load(response.body);
      info.bucketId = json.get("bucketId").get<string>();
      info.uploadUrl = json.get("uploadUrl").get<string>();
      info.authorizationToken = json.get("authorizationToken").get<string>();
      ok = true;
   }
   delete m_connection; 
   m_connection = NULL;
   return ok;
}

void BB::uploadFile(const string& bucket, const string& name, const string& type) { 

   const list<BB_Bucket>& buckets = getBuckets(false, false);
   list<BB_Bucket>::const_iterator it = std::find_if(buckets.begin(), buckets.end(), find_name(bucket)); 

   UploadUrlInfo uploadUrlInfo;
   uploadUrlInfo.bucketId = it->id;
   if (getUploadUrl(uploadUrlInfo)) { 
      ifstream fin(name.c_str(), ios::binary);
      if(!fin.is_open()) {
         cerr << "Could not read file " << name << endl;
         return;
      }

      setupConnection(uploadUrlInfo.uploadUrl); 

      uint8_t sha1[EVP_MAX_MD_SIZE];
      size_t length = computeSha1(sha1, fin);

      ostringstream sha1hex;
      sha1hex.fill('0');
      sha1hex << std::hex;

      for (uint8_t* ptr = sha1; ptr < sha1 + length; ptr++) {
         sha1hex << std::setw(2) << (unsigned int)(*ptr);
      }

      RestClient::HeaderFields headers; 
      headers["Authorization"] = uploadUrlInfo.authorizationToken;
      headers["Content-Type"] = type;
      headers["X-Bz-File-Name"] = name;
      headers["X-Bz-Content-Sha1"] = sha1hex.str();
      m_connection->SetHeaders(headers);

      fin.clear();
      fin.seekg(0, ios_base::end);
      length = fin.tellg();
      fin.seekg(0, ios_base::beg);

      char* buf = new char[length];
      fin.read(buf, length); 
      if (fin.fail()) { 
         cerr << "error: could only read " << fin.gcount() << " bytes of " << name << endl;
      }
      fin.close();

      RestClient::Response response = m_connection->post("", string(buf, length));

      delete[] buf;
   }
}

void BB::downloadFileById(const string& id, ofstream& fout) { 
   setupConnection(m_downloadUrl + API_URL_PATH);
   RestClient::Response response = m_connection->get("/b2_download_file_by_id?fileId=" + id);
   fout << response.body;
   fout.close();
}

void BB::downloadFileByName(const string& bucket, const string& name, ofstream& fout) { 
   setupConnection(m_downloadUrl + "/file");
   RestClient::Response response = m_connection->get("/" + bucket + "/" + name); 
   fout << response.body; 
   fout.close();
}

std::string BB::listBuckets() {
   if (!m_connection) { 
      std::ostringstream urlstream;
      urlstream << m_apiUrl << API_URL_PATH;
      m_connection = new RestClient::Connection(urlstream.str());
   }

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_authorizationToken;
   m_connection->SetHeaders(headers);

   std::ostringstream uristream; 
   uristream << "/b2_list_buckets?accountId=" << m_accountId;
   RestClient::Response response = m_connection->get(uristream.str());
   return response.body;
}

std::string BB::listBucket(const string& bucketId) {
   khi::Json json = khi::Json::object();
   json.set("bucketId", khi::Json::string(bucketId));
   RestClient::Response response = m_connection->post("/b2_list_file_names", json.dump());
   return response.body;
}
