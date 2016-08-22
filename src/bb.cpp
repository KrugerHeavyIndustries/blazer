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
#include <algorithm>

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

BB::BB(const string& accountId, const string& applicationKey) :
   m_accountId(accountId), 
   m_applicationKey(applicationKey),
   m_session(Session::load())
{
   RestClient::init();
}

BB::~BB() {
   RestClient::disable();
}

void BB::authorize() {
   if (m_session.unknown()) {
      RestClient::Connection* connection = connect("https://api.backblaze.com" + API_URL_PATH);
      connection->SetBasicAuth(m_accountId, m_applicationKey);

      RestClient::HeaderFields headers;
      headers["Accept"] = "application/json";
      connection->SetHeaders(headers);

      RestClient::Response response = connection->get("/b2_authorize_account");

      khi::Json json = khi::Json::load(response.body); 
      khi::Json downloadUrl = json.get("downloadUrl");
      khi::Json apiUrl = json.get("apiUrl");
      khi::Json authorizationToken = json.get("authorizationToken"); 

      m_session.apiUrl = apiUrl.get<std::string>();
      m_session.downloadUrl = downloadUrl.get<std::string>();
      m_session.authorizationToken = authorizationToken.get<std::string>();
      m_session.save();

      delete connection;
   }
}

RestClient::Connection* BB::connect(const string& baseUrl) {
   RestClient::Connection* connection = new RestClient::Connection(baseUrl);
   connection->SetTimeout(20);
   connection->SetUserAgent("cmd/blazer");
   return connection;
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
    if (refresh || m_buckets.empty())
        refreshBuckets(getContents);
    return m_buckets;
}

void BB::refreshBuckets(bool getContents) {

   RestClient::Connection* connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   m_buckets.clear();
   parseBucketsList(m_buckets, listBuckets(connection));
    
   if (getContents) {
       list<BB_Bucket>::iterator bkt;
       for (bkt = m_buckets.begin(); bkt != m_buckets.end(); ++bkt)
           getBucketContents(connection, *bkt);
   }
   delete connection;
}

void BB::getBucketContents(RestClient::Connection* connection, BB_Bucket& bucket) {
    parseObjectsList(bucket.objects, listBucket(connection, bucket.id));
}

bool BB::getUploadUrl(UploadUrlInfo& info) { 
   bool ok = false;
   RestClient::Connection* connection = connect(m_session.apiUrl + API_URL_PATH);
   khi::Json json = khi::Json::object();
   json.set("bucketId", khi::Json::string(info.bucketId));
   RestClient::Response response = connection->post("/b2_get_upload_url", json.dump());
   if (response.code == 200) {
      khi::Json json = khi::Json::load(response.body);
      info.bucketId = json.get("bucketId").get<string>();
      info.uploadUrl = json.get("uploadUrl").get<string>();
      info.authorizationToken = json.get("authorizationToken").get<string>();
      ok = true;
   }
   delete connection;
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

      uint8_t sha1[EVP_MAX_MD_SIZE];
      size_t length = computeSha1(sha1, fin);

      ostringstream sha1hex;
      sha1hex.fill('0');
      sha1hex << std::hex;

      for (uint8_t* ptr = sha1; ptr < sha1 + length; ptr++) {
         sha1hex << std::setw(2) << (unsigned int)(*ptr);
      }

      RestClient::Connection* connection = connect(uploadUrlInfo.uploadUrl);

      RestClient::HeaderFields headers; 
      headers["Authorization"] = uploadUrlInfo.authorizationToken;
      headers["Content-Type"] = type;
      headers["X-Bz-File-Name"] = name;
      headers["X-Bz-Content-Sha1"] = sha1hex.str();
      connection->SetHeaders(headers);

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

      RestClient::Response response = connection->post("", string(buf, length));

      delete[] buf;
      delete connection;
   }
}

void BB::downloadFileById(const string& id, ofstream& fout) { 
   RestClient::Connection* connection = connect(m_session.downloadUrl + API_URL_PATH);
   RestClient::Response response = connection->get("/b2_download_file_by_id?fileId=" + id);
   fout << response.body;
   fout.close();
   delete connection;
}

void BB::downloadFileByName(const string& bucket, const string& name, ofstream& fout) {
   RestClient::Connection* connection = connect(m_session.downloadUrl + "/file");
   RestClient::Response response = connection->get("/" + bucket + "/" + name);
   fout << response.body;
   fout.close();
   delete connection;
}

void BB::createBucket(const string& bucketName) { 
   RestClient::Connection* connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = connection->get("/b2_create_bucket?accountId=" + m_accountId + "&bucketName=" + bucketName + "&bucketType=allPrivate");
   if (response.code == 200) { 
      // ok
   }
   delete connection;
}

void BB::deleteBucket(const string& bucketId) {
   RestClient::Connection* connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = connection->get("/b2_delete_bucket?accountId=" + m_accountId + "&bucketId=" + bucketId);
   if (response.code == 200) { 
      // ok
   }
   delete connection;
}

std::string BB::listBuckets(RestClient::Connection* connection) {
   RestClient::Response response = connection->get("/b2_list_buckets?accountId=" + m_accountId);
   return response.body;
}

std::string BB::listBucket(RestClient::Connection* connection, const string& bucketId) {
   khi::Json json = khi::Json::object();
   json.set("bucketId", khi::Json::string(bucketId));
   RestClient::Response response = connection->post("/b2_list_file_names", json.dump());
   return response.body;
}
