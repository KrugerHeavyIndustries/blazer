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

#ifndef KHI_EXCEPTION_H
#define KHI_EXCEPTION_H

#include <string>
#include <exception>

namespace khi {

class Exception : public std::exception {

   public:
   
   explicit Exception() {}; 

   virtual ~Exception() throw() {};
};

class ResponseError : Exception {
   public:

   explicit ResponseError(int fs, const std::string& fc, const std::string& fm)
:
      m_status(fs), m_code(fc), m_message(fm) {
      build();
   };

   ~ResponseError() throw() {};

   virtual const char* what() const throw() {
      return m_what.c_str();
   }; 

   void build() {
      char tmp[4];
      snprintf(tmp, sizeof(tmp), "%d", m_status);
      m_what = std::string(tmp) + " " + m_code + " - " + m_message;
   }

   const int m_status;
   const std::string m_code;
   const std::string m_message;
   std::string m_what;
};


} // namespace khi

#endif // KHI_EXCEPTION_H
