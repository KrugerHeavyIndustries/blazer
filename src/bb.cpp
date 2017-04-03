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
#include <unistd.h>

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

   using khi::Json;
   using khi::ResponseError;

   const RestClient::Response& validate(const RestClient::Response& response) {
      if (response.code == 200) {
         return response;
      }
      const Json& json = Json::load(response.body);
      if (json.isObject()) {
         int status = json.get("status").get<int>();
         string code = json.get("code").get<string>();
         string message = json.get("message").get<string>();
         throw ResponseError(status, code, message);
      }
      throw ResponseError(response.code, "other_error", response.body);
   }
}

namespace khi {

const string BB::API_URL_PATH = "/b2api/v1";
const int BB::MINIMUM_PART_SIZE_BYTES = 100 * 1000000; // 100 MB
const int BB::MAX_FILE_PARTS = 10000;
const int BB::DEFAULT_UPLOAD_RETRY_ATTEMPTS = 5;

UploadPartTask::UploadPartTask(const BB& bb, const string& fileId, const BB_Range& range, int index, const string& filepath)
   :  Task(NULL, "upload_part_task"),
      m_bb(bb),
      m_fileId(fileId),
      m_range(range),
      m_index(index),
      m_filepath(filepath) { }

UploadPartTask::UploadPartTask(const UploadPartTask& other)
   :  Task(NULL, "upload_part_task"),
      m_bb(other.m_bb),
      m_fileId(other.m_fileId),
      m_range(other.m_range),
      m_index(other.m_index),
      m_filepath(other.m_filepath) {}

UploadPartTask::~UploadPartTask() {
}

int UploadPartTask::run() {
   int attempt = 0;
   do {
      try {
         ifstream fin(m_filepath.c_str(), ios::binary);
         if(!fin.is_open()) {
            cerr << "could not read file " <<  m_filepath << endl;
            throw std::runtime_error("could not read file " + m_filepath);
         }
         BB::UploadUrlInfo uploadUrlInfo = m_bb.getUploadPartUrl(m_fileId);
         m_hash = m_bb.uploadPart(uploadUrlInfo.uploadUrl, uploadUrlInfo.authorizationToken, m_index + 1, m_range, fin);
      } catch (const ResponseError& err) {
         if (500 /* HTTP internal server error */ <= err.m_status && err.m_status <= 599 /* HTTP network connect timeout */ && attempt < m_bb.uploadRetryAttempts()) {
            attempt++;
         } else {
            cerr << err.what() << endl;
            throw;
         }
      }
   } while (m_hash.empty());
   return EXIT_SUCCESS;
}

vector<string> UploadPartTask::map(const vector<UploadPartTask*>& uploads) {
   vector<string> hashes;
   std::transform(uploads.begin(), uploads.end(), back_inserter(hashes), result);
   return hashes;
}

string UploadPartTask::result(const UploadPartTask* task) {
   return task->hash();
}

BB::BB(const string& accountId, const string& applicationKey) :
   m_accountId(accountId),
   m_applicationKey(applicationKey),
   m_session(Session::load()),
   m_testMode(false)
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

      RestClient::Response response = validate(connection->get("/b2_authorize_account"));

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

auto_ptr<RestClient::Connection> BB::connect(const string& baseUrl) const {
   RestClient::Connection* connection = new RestClient::Connection(baseUrl);
   connection->SetUserAgent("cmd/blazer");
   connection->SetNoSignal(true);
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

const BB::UploadUrlInfo BB::getUploadUrl(const string& bucketId) const {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   Json payload = Json::object();
   payload.set("bucketId", Json::string(bucketId));
   RestClient::Response response = validate(connection->post("/b2_get_upload_url", payload.dump()));

   Json json = Json::load(response.body);

   UploadUrlInfo info;
   info.bucketOrFileId = json.get("bucketId").get<string>();
   info.uploadUrl = json.get("uploadUrl").get<string>();
   info.authorizationToken = json.get("authorizationToken").get<string>();
   return info;
}

const BB::UploadUrlInfo BB::getUploadPartUrl(const string& fileId) const {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   Json payload = Json::object();
   payload.set("fileId", Json::string(fileId));
   RestClient::Response response = validate(connection->post("/b2_get_upload_part_url", payload.dump()));

   Json json = Json::load(response.body);

   UploadUrlInfo info;
   info.bucketOrFileId = json.get("fileId").get<string>();
   info.uploadUrl = json.get("uploadUrl").get<string>();
   info.authorizationToken = json.get("authorizationToken").get<string>();
   return info;
}

int BB::uploadRetryAttempts() const {
   return DEFAULT_UPLOAD_RETRY_ATTEMPTS;
}

bool BB::useTestMode() {
   return (m_testMode = true);
}

void BB::uploadFile(const string& bucketName, const string& localFilePath, const string& remoteFileName, const string& contentType, int numThreads) {

   BB_Bucket bucket = getBucket(bucketName);

   uint64_t minimumSplitSizeBytes = MINIMUM_PART_SIZE_BYTES * 2;

   ifstream fsz(localFilePath.c_str(), ios::binary | ios::ate);
   uint64_t totalBytes = fsz.tellg();

   if (totalBytes < minimumSplitSizeBytes) {
      uploadSmall(bucket.id, localFilePath, remoteFileName, contentType, totalBytes);
   } else {
      uploadLarge(bucket.id, localFilePath, remoteFileName, contentType, totalBytes, numThreads);
   }
}

void BB::downloadFileById(const string& id, ofstream& fout) { 
   auto_ptr<RestClient::Connection> connection = connect(m_session.downloadUrl + API_URL_PATH);

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = validate(connection->get("/b2_download_file_by_id?fileId=" + id));

   fout << response.body;
   fout.close();
}

void BB::downloadFileByName(const string& bucketName, const string& remoteFileName, ofstream& fout) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.downloadUrl + "/file");

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = validate(connection->get("/" + bucketName+ "/" + remoteFileName));

   fout << response.body;
   fout.close();
}

void BB::createBucket(const string& bucketName) { 
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   validate( 
      connection->get("/b2_create_bucket?accountId=" + m_accountId + "&bucketName=" + bucketName + "&bucketType=allPrivate")
   );
}

void BB::deleteBucket(const string& bucketId) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers; 
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   validate(connection->get("/b2_delete_bucket?accountId=" + m_accountId + "&bucketId=" + bucketId));
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

   validate(connection->post("/b2_update_bucket", json.dump()));
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

   return unpackObjectsList(validate(connection->post("/b2_list_file_versions", json.dump())).body);
}

void BB::deleteFileVersion(const string& fileName, const string& fileId) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);
   
   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   Json json = Json::object();
   json.set("fileName", Json::string(fileName));
   json.set("fileId", Json::string(fileId));
   
   validate(connection->post("/b2_delete_file_version", json.dump()));
}

const BB_Object BB::getFileInfo(const string& fileId) { 
   BB_Object object;

   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);
   
   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;

   connection->SetHeaders(headers);

   Json json = Json::object();
   json.set("fileId", Json::string(fileId));

   RestClient::Response response = validate(connection->post("/b2_get_file_info", json.dump()));
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

   validate(connection->post("/b2_hide_file", json.dump()));
}

void BB::uploadSmall(const string& bucketId, const string& localFilePath, const string& remoteFileName, const string& contentType, uint64_t totalBytes) {
   ifstream fin(localFilePath.c_str(), ios::binary);
   if(!fin.is_open()) {
      throw std::runtime_error("could not read file " + localFilePath);
   }

   const UploadUrlInfo uploadUrlInfo = getUploadUrl(bucketId);

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
   if (m_testMode) {
      headers["X-Bz-Test-Mode"] = "fail_some_uploads";
   }
   connection->SetHeaders(headers);

   fin.clear();
   fin.seekg(0, ios_base::beg);

   char* buf = new char[totalBytes];
   fin.read(buf, totalBytes);
   if (fin.fail()) {
      throw std::runtime_error("could not read all of " + localFilePath);
   }
   fin.close();

   RestClient::Response response = connection->post("", string(buf, totalBytes));
   delete[] buf;
   validate(response);
}

void BB::uploadLarge(const string& bucketId, const string& localFilePath, const string& remoteFileName, const string& contentType, uint64_t totalBytes, int numThreads) {
   ifstream fin(localFilePath.c_str(), ios::binary);
   if(!fin.is_open()) {
      throw std::runtime_error("could not read file " + localFilePath);
   }

   string fileId = startLargeFile(bucketId, remoteFileName, contentType);
   vector<BB_Range> ranges = choosePartRanges(totalBytes, MINIMUM_PART_SIZE_BYTES);

   Dispatcho dispatcho(numThreads);

   int index = 0;
   vector<UploadPartTask*> uploads;
   for (vector<BB_Range>::const_iterator iter = ranges.begin(); iter != ranges.end(); ++iter) {
      uploads.push_back(new UploadPartTask(*this, fileId, *iter, index++, localFilePath));
      dispatcho.async(uploads.back());
   }

   while (dispatcho.size() > 0) {
      sleep(0);
   }

   dispatcho.stop();

   finishLargeFile(fileId, UploadPartTask::map(uploads));

   for (vector<UploadPartTask*>::iterator iter = uploads.begin(); iter != uploads.end(); ++iter) {
      delete (*iter);
   }
}

string BB::startLargeFile(const string& bucketId, const string& fileName, const string& contentType) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   headers["Content-Type"] = "application/json";
   connection->SetHeaders(headers);

   Json json = Json::object();
   json.set("bucketId", Json::string(bucketId));
   json.set("fileName", Json::string(fileName));
   json.set("contentType", Json::string(contentType));

   RestClient::Response response = connection->post("/b2_start_large_file", json.dump());
   validate(response);
   return Json::load(response.body).get("fileId").get<string>();
}

string BB::uploadPart(const string& uploadUrl, const string& authorizationToken, int partNumber, const BB_Range& range, ifstream& fs) const {
   auto_ptr<RestClient::Connection> connection = connect(uploadUrl);

   uint8_t sha1[EVP_MAX_MD_SIZE];
   size_t length = computeSha1UsingRange(sha1, fs, range.start, range.end);

   ostringstream sha1hex;
   sha1hex.fill('0');
   sha1hex << std::hex;

   for (uint8_t* ptr = sha1; ptr < sha1 + length; ptr++) {
      sha1hex << std::setw(2) << (unsigned int)(*ptr);
   }

   ostringstream convertPartNumber;
   convertPartNumber << partNumber;

   ostringstream convertLength;
   convertLength << range.length();

   RestClient::HeaderFields headers;
   headers["Authorization"] = authorizationToken;
   //headers["Content-Type"] = "application/json; charset=utf-8";
   headers["X-Bz-Part-Number"] = convertPartNumber.str();
   headers["X-Bz-Content-Sha1"] = sha1hex.str();
   if (m_testMode) {
      headers["X-Bz-Test-Mode"] = "fail_some_uploads";
   }
   headers["Content-Length"] = convertLength.str();

   connection->SetHeaders(headers);

   fs.clear();
   fs.seekg(range.start, ios_base::beg);

   char* buf = new char[range.length()];
   fs.read(buf, range.length());
   if (fs.fail()) {
      throw std::runtime_error("could not read file segment");
   }
   fs.close();

   RestClient::Response response = connection->post("", string(buf, range.length()));
   delete[] buf;
   validate(response);

   return sha1hex.str();
}

void BB::finishLargeFile(const string& fileId, const vector<string>& hashes) {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   headers["Content-Type"] = "application/json";
   connection->SetHeaders(headers);

   Json json = Json::object();
   json.set("fileId", Json::string(fileId));

   Json array = Json::array();
   for (vector<string>::const_iterator iter = hashes.begin(); iter != hashes.end(); ++iter) {
      array.append(Json::string(*iter));
   }
   json.set("partSha1Array", array);

   validate(connection->post("/b2_finish_large_file", json.dump()));
}

vector<BB_Range> BB::choosePartRanges(uint64_t totalBytes, uint64_t minimumPartBytes) {
   vector<BB_Range> ranges;
   uint64_t n = std::min(totalBytes / minimumPartBytes, static_cast<uint64_t>(MAX_FILE_PARTS));
   uint64_t minus1 = n - 1;
   uint64_t partBytes = totalBytes / n;
   for (int i = 0; i < n; ++i) {
      ranges.push_back(BB_Range(i * partBytes, (i < minus1 ? (i + 1) * partBytes : totalBytes) - 1));
   }
   return ranges;
}

list<BB_Bucket> BB::listBuckets() {
   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   RestClient::Response response = validate(connection->get("/b2_list_buckets?accountId=" + m_accountId));
   return unpackBucketsList(response.body);
}

list<BB_Object> BB::listBucket(const string& bucketName, const string& startFileName, int maxFileCount) {
   const int maxFileCountDefaults[2] = { 0, 100 }; // 0 means show 100, 100 means show 100
   const int maxFileCountLimit = 1000;

   auto_ptr<RestClient::Connection> connection = connect(m_session.apiUrl + API_URL_PATH);

   RestClient::HeaderFields headers;
   headers["Authorization"] = m_session.authorizationToken;
   connection->SetHeaders(headers);

   Json json = Json::object();
   json.set("bucketId", Json::string(getBucket(bucketName).id));
   if (startFileName.size() > 0) {
      json.set("startFileName", Json::string(startFileName));
   }
   if (maxFileCount != maxFileCountDefaults[0] && maxFileCount != maxFileCountDefaults[1]) {
      maxFileCount = std::max(maxFileCount, maxFileCountLimit);
      json.set("maxFileCount", Json::integer(maxFileCount));
   }
   RestClient::Response response = validate(connection->post("/b2_list_file_names", json.dump()));
   return unpackObjectsList(response.body);
}

} // namespace khi
