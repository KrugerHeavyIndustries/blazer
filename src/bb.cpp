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

#include "restclient-cpp/connection.h"
#include "restclient-cpp/restclient.h"

#include "bb.h"
#include "coding.h"
#include "jsoncpp.h"
#include "exceptions.h"
   
using namespace std;

namespace {
   using khi::BB_Bucket;
   struct find_name : std::unary_function<BB_Bucket, bool> {
      std::string name;
      find_name(const std::string& value) : name(value) {}
      bool operator()(const BB_Bucket& b) const {
         return b.name == name;
      }
   };
}

namespace khi {

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
      auto_ptr<RestClient::Connection> connection = connect("https://api.backblaze.com" + API_URL_PATH);
      connection->SetBasicAuth(m_accountId, m_applicationKey);

      RestClient::HeaderFields headers;
      headers["Accept"] = "application/json";
      connection->SetHeaders(headers);

      RestClient::Response response = connection->get("/b2_authorize_account");
      if (response.code != 200) {
         throwResponseError(Json::load(response.body));
      }

      Json json = Json::load(response.body);
      Json downloadUrl = json.get("downloadUrl");
      Json apiUrl = json.get("apiUrl");
      Json authorizationToken = json.get("authorizationToken");

      m_session.apiUrl = apiUrl.get<std::string>();
      m_session.downloadUrl = downloadUrl.get<std::string>();
      m_session.authorizationToken = authorizationToken.get<std::string>();
      m_session.save();
   }
}

auto_ptr<RestClient::Connection> BB::connect(const string& baseUrl) {
   RestClient::Connection* connection = new RestClient::Connection(baseUrl);
   connection->SetTimeout(20);
   connection->SetUserAgent("cmd/blazer");
   return auto_ptr<RestClient::Connection>(connection);
}

list<BB_Bucket> BB::unpackBucketsList(const string& json) {
   list<BB_Bucket> buckets;
   Json root = Json::load(json); 
   if (root.isObject()) { 
      Json array = root.get("buckets");
      if (array.isArray()) { 
         for (int i = 0; i < array.size(); ++i) { 
            Json elem = array.at(i);
            if (elem.isObject()) { 
               Json bucketId = elem.get("bucketId"); 
               Json bucketName = elem.get("bucketName"); 
               Json bucketType = elem.get("bucketType"); 
               buckets.push_back(BB_Bucket(bucketId.get<string>(), bucketName.get<string>(), bucketType.get<string>())); 
            }
         }
      }
   }
   return buckets;
}

list<BB_Object> BB::unpackObjectsList(const string& json) {
   list<BB_Object> files;
   Json root = Json::load(json); 
   if (root.isObject()) { 
      Json array = root.get("files");
      if (array.isArray()) { 
         for (int i = 0; i < array.size(); ++i) { 
           Json elem = array.at(i);
            if (elem.isObject()) { 
               files.push_back(unpackObject(elem));
            }
         }
      }
      Json nextFileName = root.get("nextFileName");
   }
   return files;
}

BB_Object BB::unpackObject(const Json& json) {
   BB_Object object;
   Json action = json.get("action");
   if (action.isString())
      object.action = action.get<string>();

   Json contentLength = json.get("contentLength");
   if (contentLength.isInteger())
      object.contentLength = contentLength.get<int>();

   Json contentType = json.get("contentType");
   if (contentType.isString())
      object.contentType = contentType.get<string>();

   Json contentSha1 = json.get("contentSha1");
   if (contentSha1.isString())
      object.contentSha1 = contentSha1.get<string>();

   Json fileId = json.get("fileId"); 
   if (fileId.isString())
      object.id = fileId.get<string>();

   Json fileName = json.get("fileName");
   if (fileName.isString()) 
      object.name = fileName.get<string>();

   Json uploadTimestamp = json.get("uploadTimestamp");
   if (uploadTimestamp.isInteger()) 
      object.uploadTimestamp = uploadTimestamp.get<int>();

   return object;
}

void BB::throwResponseError(const Json& json) {
   int status = json.get("status").get<int>();
   string code = json.get("code").get<string>();
   string message = json.get("message").get<string>();
   throw ResponseError(status, code, message);
}

std::list<BB_Bucket>& BB::getBuckets(bool getContents, bool refresh) {
    if (refresh || m_buckets.empty())
        refreshBuckets(getContents);
    return m_buckets;
}

BB_Bucket BB::getBucket(const string& bucketName) {
   const std::list<BB_Bucket>& buckets = getBuckets(false, false);
   list<BB_Bucket>::const_iterator it = std::find_if(buckets.begin(), buckets.end(), find_name(bucketName)); 
   if (it == buckets.end()) { 
      throw std::runtime_error("non existent bucket");
   }
   return (*it);
}

void BB::refreshBuckets(bool getContents) {
   m_buckets = listBuckets();
   if (getContents) {
      for (list<BB_Bucket>::iterator bkt = m_buckets.begin(); bkt != m_buckets.end(); ++bkt) { 
         (*bkt).objects = listBucket((*bkt).name);
      }
   }
}

bool BB::getUploadUrl(UploadUrlInfo& info) { 
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);
   Json payload = Json::object();
   payload.set("bucketId", Json::string(info.bucketId));
   RestClient::Response response = connection->post("/b2_get_upload_url", payload.dump());
   if (response.code != 200) {
      throwResponseError(Json::load(response.body)); 
   }
   Json json = Json::load(response.body);
   info.bucketId = json.get("bucketId").get<string>();
   info.uploadUrl = json.get("uploadUrl").get<string>();
   info.authorizationToken = json.get("authorizationToken").get<string>();
   return true;
}

void BB::uploadFile(const string& bucketName, const string& localFilePath, const string& remoteFileName, const string& contentType) { 

   BB_Bucket bucket = getBucket(bucketName);

   UploadUrlInfo uploadUrlInfo;
   uploadUrlInfo.bucketId = bucket.id;
   if (getUploadUrl(uploadUrlInfo)) { 
      ifstream fin(localFilePath.c_str(), ios::binary);
      if(!fin.is_open()) {
         throw std::runtime_error("could not read file " + localFilePath);
      }

      uint8_t sha1[EVP_MAX_MD_SIZE];
      size_t length = computeSha1(sha1, fin);

      ostringstream sha1hex;
      sha1hex.fill('0');
      sha1hex << std::hex;

      for (uint8_t* ptr = sha1; ptr < sha1 + length; ptr++) {
         sha1hex << std::setw(2) << (unsigned int)(*ptr);
      }

      auto_ptr<RestClient::Connection> connection = connect(uploadUrlInfo.uploadUrl);

      RestClient::HeaderFields headers; 
      headers["Authorization"] = uploadUrlInfo.authorizationToken;
      headers["Content-Type"] = contentType;
      headers["X-Bz-File-Name"] = remoteFileName;
      headers["X-Bz-Content-Sha1"] = sha1hex.str();
      connection->SetHeaders(headers);

      fin.clear();
      fin.seekg(0, ios_base::end);
      length = fin.tellg();
      fin.seekg(0, ios_base::beg);

      char* buf = new char[length];
      fin.read(buf, length); 
      if (fin.fail()) { 
         throw std::runtime_error("could not read all of " + localFilePath);
      }
      fin.close();

      RestClient::Response response = connection->post("", string(buf, length));
      delete[] buf;
      if (response.code != 200) { 
         throwResponseError(Json::load(response.body));
      }
   }
}

void BB::downloadFileById(const string& id, ofstream& fout) { 
   auto_ptr<RestClient::Connection> connection = connect(m_session.downloadUrl + API_URL_PATH);

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = connection->get("/b2_download_file_by_id?fileId=" + id);
   if (response.code != 200) { 
      throwResponseError(Json::load(response.body));
   }
   fout << response.body;
   fout.close();
}

void BB::downloadFileByName(const string& bucketName, const string& remoteFileName, ofstream& fout) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.downloadUrl + "/file");

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = connection->get("/" + bucketName+ "/" + remoteFileName);
   if (response.code != 200) {
      throwResponseError(Json::load(response.body));
   }
   fout << response.body;
   fout.close();
}

void BB::createBucket(const string& bucketName) { 
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = connection->get("/b2_create_bucket?accountId=" + m_accountId + "&bucketName=" + bucketName + "&bucketType=allPrivate");
   if (response.code != 200) { 
      throwResponseError(Json::load(response.body));
   }
}

void BB::deleteBucket(const string& bucketId) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = connection->get("/b2_delete_bucket?accountId=" + m_accountId + "&bucketId=" + bucketId);
   if (response.code != 200) { 
      throwResponseError(Json::load(response.body));
   }
}

void BB::updateBucket(const string& bucketId, const string& bucketType) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);
   
   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   headers["Content-Type"] = "application/json";
   connection->SetHeaders(headers);

   Json json = Json::object();
   json.set("accountId", Json::string(m_accountId));
   json.set("bucketId", Json::string(bucketId));
   json.set("bucketType", Json::string(bucketType));

   RestClient::Response response = connection->post("/b2_update_bucket", json.dump());
   if (response.code != 200) { 
      throwResponseError(Json::load(response.body));
   }
}

std::list<BB_Object> BB::listFileVersions(const string& bucketId, const string& startFileName, const string& startFileId, int maxFileCount) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   headers["Content-Type"] = "application/json";
   connection->SetHeaders(headers);

   Json json = Json::object();
   json.set("bucketId", Json::string(bucketId));
   if (!startFileName.empty()) {
      json.set("startFileName", Json::string(startFileName));
      if (!startFileId.empty()) {
         json.set("startFileId", Json::string(startFileId));
      }
   }
   json.set("maxFileCount", Json::integer(maxFileCount));

   RestClient::Response response = connection->post("/b2_list_file_versions", json.dump());
   if (response.code != 200) {
      throwResponseError(Json::load(response.body));
   }
   return unpackObjectsList(response.body);
}

void BB::deleteFileVersion(const string& fileName, const string& fileId) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);
   
   Json json = Json::object();
   json.set("fileName", Json::string(fileName));
   json.set("fileId", Json::string(fileId));
   
   RestClient::Response response = connection->post("/b2_delete_file_version", json.dump());
   if (response.code != 200) {
      throwResponseError(Json::load(response.body));
   }
}

const BB_Object BB::getFileInfo(const string& fileId) { 
   BB_Object object;

   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);
   
   Json json = Json::object();
   json.set("fileId", Json::string(fileId));

   RestClient::Response response = connection->post("/b2_get_file_info", json.dump());
   if (response.code != 200) {
      throwResponseError(Json::load(response.body));
   }
   Json obj = Json::load(response.body);
   if (obj.isObject()) {
      object = unpackObject(obj);
   }
   return object;
}

void BB::hideFile(const string& bucketId, const string& fileName) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   Json json = Json::object();
   json.set("bucketId", Json::string(bucketId));
   json.set("fileName", Json::string(fileName));

   RestClient::Response response = connection->post("/b2_hide_file", json.dump());
   if (response.code != 200) {
      throwResponseError(Json::load(response.body));
   }
}

list<BB_Bucket> BB::listBuckets() {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = connection->get("/b2_list_buckets?accountId=" + m_accountId);
   if (response.code != 200) {
      throwResponseError(Json::load(response.body));
   }
   return unpackBucketsList(response.body);
}

list<BB_Object> BB::listBucket(const string& bucketName, const string& folderName) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   Json json = Json::object();
   json.set("bucketId", Json::string(getBucket(bucketName).id));
   RestClient::Response response = connection->post("/b2_list_file_names", json.dump());
   if (response.code != 200) { 
      throwResponseError(Json::load(response.body));
   }
   return unpackObjectsList(response.body);
}

} // namespace khi
