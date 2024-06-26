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
#include <memory>
#include <stdint.h>

#include "multidict.h"
#include "session.h"
#include "dispatcho.h"

namespace RestClient { 
   class Connection;
}

namespace khi {

class Json;

struct BB_Object { 
   std::string id;
   std::string name;
   uint64_t contentLength;
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

struct BB_Range {
   uint64_t start;
   uint64_t end;

   inline BB_Range(uint64_t s, uint64_t e) : start(s), end(e) {}
   inline BB_Range(const BB_Range& other) : start(other.start), end(other.end) {}

   uint64_t length() const { return static_cast<uint64_t>(end - start) + 1; }
};

class BB;

class UploadPartTask : public Task {

   public:

   UploadPartTask(const BB& bb, const std::string& fileId, const BB_Range& range, int index, const std::string& filepath);
   UploadPartTask(const UploadPartTask&);

   virtual ~UploadPartTask();

   virtual int run();

   inline std::string hash() const {
      return m_hash;
   }

   static std::vector<std::string> map(const std::vector<UploadPartTask*>& uploads);
   static std::string result(const UploadPartTask* task);

   private:

   UploadPartTask& operator=(const UploadPartTask&); // prevent assign

   const BB& m_bb;
   const std::string& m_fileId;
   const BB_Range& m_range;
   const int m_index;
   const std::string& m_filepath;
   std::string m_hash;
};

class DownloadPartTask : public Task {

   public:

   DownloadPartTask(const BB& bb, const std::string& authorizationToken, const std::string& downloadUrl, const BB_Range& range, int index, const std::string& filepath);
   DownloadPartTask(const DownloadPartTask&);

   virtual ~DownloadPartTask();

   virtual int run();

   static const std::string filepart(const std::string& path, int index);
   static void coalesce(const std::string& filepath, const std::vector<DownloadPartTask*>& downloads);
   static void cleanup(const std::string& filepath, const std::vector<DownloadPartTask*>& downloads);

   private:

   DownloadPartTask& operator=(const DownloadPartTask&); // prevent assign

   static const std::string downloadPath(const std::string& filepath);

   const BB& m_bb;
   const std::string& m_authorizationToken;
   const std::string& m_downloadUrl;
   const BB_Range& m_range;
   const int m_index;
   const std::string& m_filepath;
   std::string m_result;
};

class BB {

   friend class UploadPartTask;
   friend class DownloadPartTask;

   const std::string m_accountId;
   const std::string m_applicationKey;
   
   Session m_session;

   std::list<BB_Bucket> m_buckets;

   bool m_testMode;

   static const std::string API_URL_PATH;
   static const int MINIMUM_PART_SIZE_BYTES;
   static const int MINIMUM_SPLIT_SIZE_BYTES;
   static const int MAX_FILE_PARTS;
   static const int DEFAULT_UPLOAD_RETRY_ATTEMPTS;
    
   static std::list<BB_Bucket> unpackBucketsList(const std::string& json);

   static std::list<BB_Object> unpackObjectsList(const std::string& json);

   static BB_Object unpackObject(const Json& json);

   public:

   BB(const std::string& accountId, const std::string& applicationKey, bool testMode = false);
   ~BB();

   void authorize();

   std::list<BB_Bucket>& getBuckets(bool getContents, bool refresh);

   BB_Bucket getBucket(const std::string& bucketName); 
                                     
   void refreshBuckets(bool getContents);
    
   int uploadFile(const std::string& bucketName, const std::string& localFileName, const std::string& remoteFileName, const std::string& contentType, int numThreads = 1);
   
   int downloadFileById(const std::string& fileId, const std::string& localFilePath, int numThreads = 1);
     
   int downloadFileByName(const std::string& bucketName, const std::string& remoteFileName, std::ofstream& fout, int numThreads = 1);

   void deleteFileVersion(const std::string& fileName, const std::string& fileId);

   void createBucket(const std::string& bucketName);
   
   void deleteBucket(const std::string& bucketId);

   void updateBucket(const std::string& bucketId, const std::string& bucketType);

   std::list<BB_Object> listFileVersions(const std::string& bucketId, const std::string& startFileName = "", const std::string& startFileId = "", int maxFileCount = 0);
   
   std::list<BB_Object> listBucket(const std::string& bucketName, const std::string& startFileName = "", int maxFileCount = 100);

   const BB_Object getFileInfo(const std::string& fileId);

   void hideFile(const std::string& bucketName, const std::string& fileName);
   struct UploadUrlInfo {
      std::string bucketOrFileId;
      std::string uploadUrl;
      std::string authorizationToken;
   };

   const UploadUrlInfo getUploadUrl(const std::string& bucketId) const;

   const UploadUrlInfo getUploadPartUrl(const std::string& fileId) const;

   int uploadRetryAttempts() const;

   bool useTestMode();

   private:

   int uploadSmall(const std::string& bucketId, const std::string& localFilePath, const std::string& remoteFileName, const std::string& contentType, uint64_t totalBytes);

   int uploadLarge(const std::string& buckedId, const std::string& localFilePath, const std::string& remoteFileName, const std::string& contentType, uint64_t totalBytes, int numThreads = 1);

   std::string startLargeFile(const std::string& bucketId, const std::string& fileName, const std::string& contentType);

   std::string uploadPart(const std::string& uploadUrl, const std::string& authorizationToken, int partNumber, const BB_Range& range, std::ifstream& fs) const;

   std::string downloadPart(const std::string& downloadUrl, const std::string& authorizationToken, int partNumber, const BB_Range& range, std::ofstream& fs) const;

   void finishLargeFile(const std::string& fileId, const std::vector<std::string>& partsSha1);

   std::vector<BB_Range> choosePartRanges(uint64_t totalBytes);

   std::string rangeHeader(const BB_Range& range) const;

   std::unique_ptr<RestClient::Connection> connect(const std::string& baseUrl) const;
 
   std::list<BB_Bucket> listBuckets();
};

} // namespace khi
#endif // BLAZER_IO_H
