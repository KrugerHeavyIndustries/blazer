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

namespace khi {

using namespace std;

// types from http://www.iana.org/assignments/media-types/

MimeTypes::Dictionary MimeTypes::ms_mimeTypes; 

void MimeTypes::initialize() {

    ms_mimeTypes[".txt"] = "text/plain";
    ms_mimeTypes[".pov"] = "text/plain";
    ms_mimeTypes[".inc"] = "text/plain";
    ms_mimeTypes[".sh"] = "text/plain";
    ms_mimeTypes[".rb"] = "text/plain";
    ms_mimeTypes[".erb"] = "text/plain";
    ms_mimeTypes[".h"] = "text/plain";
    ms_mimeTypes[".hh"] = "text/plain";
    ms_mimeTypes[".hpp"] = "text/plain";
    ms_mimeTypes[".cpp"] = "text/plain";
    ms_mimeTypes[".c"] = "text/plain";
    ms_mimeTypes[".mak"] = "text/plain";
    ms_mimeTypes["Makefile"] = "text/plain";
    
    ms_mimeTypes[".css"] = "text/css";
    ms_mimeTypes[".csv"] = "text/csv";
    ms_mimeTypes[".htm"] = "text/html";
    ms_mimeTypes[".html"] = "text/html";
    ms_mimeTypes[".xml"] = "text/xml";
    
    ms_mimeTypes[".png"] = "image/png";
    ms_mimeTypes[".gif"] = "image/gif";
    ms_mimeTypes[".jpg"] = "image/jpeg";
    ms_mimeTypes[".jpeg"] = "image/jpeg";
    ms_mimeTypes[".tiff"] = "image/tiff";
    ms_mimeTypes[".svg"] = "image/svg+xml";
    ms_mimeTypes[".tga"] = "image";
    
    ms_mimeTypes[".mp3"] = "audio/mp3";
    
    ms_mimeTypes[".mp4"] = "video/mp4";
    ms_mimeTypes[".mpg"] = "video/mpeg";
    ms_mimeTypes[".mpeg"] = "video/mpeg";
    ms_mimeTypes[".mov"] = "video/quicktime";
    
    ms_mimeTypes[".tex"] = "application/x-latex";
    ms_mimeTypes[".pdf"] = "application/pdf";
    
    ms_mimeTypes[".tar"] = "application/x-tar";
    ms_mimeTypes[".gz"] = "application/octet-stream";
    ms_mimeTypes[".zip"] = "application/zip";
    
    ms_mimeTypes[".js"] = "application/js";
}

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
    
    return string("");
} 

} // namespace khi
