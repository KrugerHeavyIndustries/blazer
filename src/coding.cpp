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

#include "coding.h"
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <cmath>


using namespace std;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
namespace {
   // OpenSSL 1.1.0 and up define new EVP digest routines.
   // These anonymous namespaced versions are for compatability with
   // previous versions
   EVP_MD_CTX* EVP_MD_CTX_new() {
      void* ret = OPENSSL_malloc(sizeof(EVP_MD_CTX));
      if (ret != NULL) {
         memset(ret, 0, sizeof(EVP_MD_CTX));
      }
      return static_cast<EVP_MD_CTX*>(ret);
   }

   void EVP_MD_CTX_free(EVP_MD_CTX* ctx) {
        EVP_MD_CTX_cleanup(ctx);
        OPENSSL_free(ctx);
   }
}  // namespace
#endif

// Encode binary data in ASCII form using base 64
std::string encodeB64(uint8_t * data, size_t dataLen)
{
    // http://www.ioncannon.net/programming/34/howto-base64-encode-with-cc-and-openssl/
    BIO * b64 = BIO_new(BIO_f_base64());
    BIO * bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, data, dataLen);
    BIO_ctrl(b64, BIO_CTRL_FLUSH, 0, NULL);//BIO_flush(b64);
    
    BUF_MEM * bptr;
    BIO_get_mem_ptr(b64, &bptr);
    
    string b64string(bptr->data, bptr->length-1);
//    cout << "b64string: \"" << b64string << "\"" << endl;
    BIO_free_all(b64);
   return b64string;
}

// Compute a MD5 checksum of a given data stream as a binary string
// openssl dgst -md5 -binary FILE | openssl enc -base64
const streamsize kMD5_ChunkSize = 16384;
const char * hexchars = "0123456789abcdef";
size_t computeMD5(uint8_t md5[EVP_MAX_MD_SIZE], std::istream& istrm)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit(ctx, EVP_md5());
    
    uint8_t * buf = new uint8_t[kMD5_ChunkSize];
    while (istrm) {
        istrm.read((char*)buf, kMD5_ChunkSize);
        streamsize count = istrm.gcount();
        EVP_DigestUpdate(ctx, buf, count);
    }
    delete[] buf;
    
    unsigned int mdLen;
    EVP_DigestFinal_ex(ctx, md5, &mdLen);
    EVP_MD_CTX_free(ctx);
    return mdLen;
}

// Compute a MD5 checksum of a given data stream as a hex-encoded ASCII string
std::string computeMD5(std::istream& istrm)
{
    uint8_t md5[EVP_MAX_MD_SIZE];
    size_t mdLen = computeMD5(md5, istrm);
    std::ostringstream md5strm;
    for(size_t j = 0; j < mdLen; ++j)
        md5strm << hexchars[(md5[j] >> 4) & 0x0F] << hexchars[md5[j] & 0x0F];
    
    return md5strm.str();
}

size_t computeSha1(uint8_t sha1[EVP_MAX_MD_SIZE], std::istream& stream) {
   stream.clear();
   stream.seekg(0, ios::end);
   uint64_t end = stream.tellg();
   stream.seekg(0, ios::beg);
   return computeSha1UsingRange(sha1, stream, 0, end);
}

size_t computeSha1UsingRange(uint8_t sha1[EVP_MAX_MD_SIZE], std::istream& stream, uint64_t firstByte, uint64_t lastByte) {
   unsigned int length;

   EVP_MD_CTX* ctx = EVP_MD_CTX_new();
   EVP_DigestInit(ctx, EVP_sha1());

   stream.seekg(firstByte, ios::beg);

   uint64_t remainBytes = (lastByte - firstByte) + 1;

   uint8_t* buf = (uint8_t*)alloca(kMD5_ChunkSize);
   while (stream && remainBytes > 0) {
      stream.read((char*)buf, std::min(static_cast<uint64_t>(kMD5_ChunkSize), remainBytes));
      streamsize count = stream.gcount();
      EVP_DigestUpdate(ctx, buf, count);
      remainBytes -= count;
   }

   EVP_DigestFinal_ex(ctx, sha1, &length);
   EVP_MD_CTX_free(ctx);
   return length;
}

string computeSHA1(std::istream& fin) { 
   uint8_t sha1[EVP_MAX_MD_SIZE];
   computeSha1(sha1, fin);
   return string((char*)sha1);
}
