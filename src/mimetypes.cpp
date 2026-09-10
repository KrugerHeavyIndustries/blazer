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
   
#include "mimetypes.h"

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>

namespace khi {

using namespace std;

// types from http://www.iana.org/assignments/media-types/

MimeTypes::Dictionary MimeTypes::ms_mimeTypes = {
    {".txt", "text/plain"},
    {".pov", "text/plain"},
    {".inc", "text/plain"},
    {".sh", "text/plain"},
    {".rb", "text/plain"},
    {".erb", "text/plain"},
    {".h", "text/plain"},
    {".hh", "text/plain"},
    {".hpp", "text/plain"},
    {".cpp", "text/plain"},
    {".c", "text/plain"},
    {".mak", "text/plain"},
    {"Makefile", "text/plain"},
    
    {".css", "text/css"},
    {".csv", "text/csv"},
    {".htm", "text/html"},
    {".html", "text/html"},
    {".xml", "text/xml"},
    
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".tiff", "image/tiff"},
    {".svg", "image/svg+xml"},
    {".tga", "image/tga"},
    {".mp3", "audio/mp3"},
    {".mp4", "video/mp4"},
    {".mpg", "video/mpeg"},
    {".mpeg", "video/mpeg"},
    {".mov", "video/quicktime"},
    {".tex", "application/x-latex"},
    {".pdf", "application/pdf"},
    {".tar", "application/x-tar"},
    {".bz", "application/x-bzip"},
    {".bz2", "application/x-bzip2"},
    {".gz", "application/x-gzip"},
    {".zip", "application/zip"},
    {".js", "application/js"}
}; 

string MimeTypes::matchByExtension(const string& filename) {
   string::size_type demark = filename.find_last_of('.');
   if (demark != string::npos) {// File name has extension
      string extension = filename.substr(demark);
      transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
      if (ms_mimeTypes.find(extension) != ms_mimeTypes.end())
         return ms_mimeTypes[extension];
   }
    
   if (ms_mimeTypes.find(filename) != ms_mimeTypes.end())
       return ms_mimeTypes[filename];
    
   return string("application/octet-stream"); // default to
} 

} // namespace khi
