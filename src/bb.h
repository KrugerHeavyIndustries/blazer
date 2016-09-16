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

struct BB_Range {
   long start;
   long end;

   inline BB_Range(long s, long e) : start(s), end(e) {}
   inline BB_Range(const BB_Range& other) : start(other.start), end(other.end) {}

   int length() const { return static_cast<int>(end - start) + 1; }
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

class BB {

   friend class UploadPartTask;

   std::string m_accountId;
   std::string m_applicationKey;
   
   Session m_session;

   std::list<BB_Bucket> m_buckets;

   static const std::string API_URL_PATH;
   static const int MINIMUM_PART_SIZE_BYTES;
   static const int MAX_FILE_PARTS;
    
   static std::list<BB_Bucket> unpackBucketsList(const std::string& json);

   static std::list<BB_Object> unpackObjectsList(const std::string& json);

   static BB_Object unpackObject(const Json& json);

   static void throwResponseError(const Json& json);
   
    
   public:

   BB(const std::string& accountId, const std::string& applicationKey);
   ~BB();

   void authorize();

   std::list<BB_Bucket>& getBuckets(bool getContents, bool refresh);

   BB_Bucket getBucket(const std::string& bucketName); 
                                     
   void refreshBuckets(bool getContents);
    
   void uploadFile(const std::string& bucketName, const std::string& localFileName, const std::string& remoteFileName, const std::string& contentType);
   
   void downloadFileById(const std::string& fileId, std::ofstream& fout);
     
   void downloadFileByName(const std::string& bucketName, const std::string& remoteFileName, std::ofstream& fout);

   void deleteFileVersion(const std::string& fileName, const std::string& fileId);

   void createBucket(const std::string& bucketName);
   
   void deleteBucket(const std::string& bucketId);

   void updateBucket(const std::string& bucketId, const std::string& bucketType);

   std::list<BB_Object> listFileVersions(const std::string& bucketId, const std::string& startFileName = "", const std::string& startFileId = "", int maxFileCount = 0);
   
   std::list<BB_Object> listBucket(const std::string& bucketName, const std::string& folderName = "");

   const BB_Object getFileInfo(const std::string& fileId);

   void hideFile(const std::string& bucketName, const std::string& fileName);
   struct UploadUrlInfo {
      std::string bucketOrFileId;
      std::string uploadUrl;
      std::string authorizationToken;
   };

   const UploadUrlInfo getUploadUrl(const std::string& bucketId) const;

   const UploadUrlInfo getUploadPartUrl(const std::string& fileId) const;

   private:

   void uploadSmall(const std::string& bucketId, const std::string& localFilePath, const std::string& remoteFileName, const std::string& contentType, long totalBytes);

   void uploadLarge(const std::string& buckedId, const std::string& localFilePath, const std::string& remoteFileName, const std::string& contentType, long totalBytes);

   std::string startLargeFile(const std::string& bucketId, const std::string& fileName, const std::string& contentType);

   std::string uploadPart(const std::string& uploadUrl, const std::string& authorizationToken, int partNumber, const BB_Range& range, std::ifstream& fs) const;

   void finishLargeFile(const std::string& fileId, const std::vector<std::string>& partsSha1);

   std::vector<BB_Range> choosePartRanges(long totalBytes, long minimumPartBytes);

   std::auto_ptr<RestClient::Connection> connect(const std::string& baseUrl) const;
 
   std::list<BB_Bucket> listBuckets();
};

} // namespace khi
#endif // BLAZER_IO_H
