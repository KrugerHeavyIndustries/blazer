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

#ifndef JSON_H
#define JSON_H

#include <string>

struct json_t; 

namespace khi {

class Json { 
   public: 

      Json(const Json& json);

      ~Json();

      Json& operator=(const Json& json);
   
      bool isObject() const;

      bool isArray() const; 

      bool isString() const;

      bool isInteger() const;

      bool isReal() const;

      bool isBoolean() const;

      std::string dump();

      template<typename ValueType>
      ValueType get() const
      { 
         return internalGet(static_cast<ValueType*>(NULL));
      }
      
      Json get(const std::string& key) const;

      Json at(int index) const;

      void set(const std::string& key, const Json& json);

      int size() const;

      static Json load(const std::string&);

      static Json object();

      static Json string(const std::string& value);

      static Json array();

   private: 

      Json(); 

      Json(json_t* json); 

      bool internalGet(bool*) const; 

      double internalGet(double*) const; 

      int internalGet(int*) const; 

      uint64_t internalGet(uint64_t*) const; 

      std::string internalGet(std::string*) const;

      json_t* m_json;
};

} // namespace khi
#endif // JSON_H
