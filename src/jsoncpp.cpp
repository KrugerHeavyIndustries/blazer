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
   
#include "jsoncpp.h"

#include <stdexcept>

#include <jansson.h>

namespace khi {

Json::Json() : m_json(NULL) {
}

Json::Json(json_t* json)
   : m_json(json) {
}

Json::Json(const Json& json)
   : m_json(json.m_json) {
   if (m_json) {
      json_incref(m_json);
   }
}

Json::~Json() {
   if (m_json) {
      json_decref(m_json);
   }
}

Json& Json::operator=(const Json& json) {
   json_t* old = m_json;
   m_json = json.m_json;
   if (m_json) {
      json_incref(m_json);
   }
   if (old) {
      json_decref(old);
   }
   return (*this);
}

Json Json::load(const std::string& json) { 
   json_error_t error;
   return Json(json_loads(json.c_str(), 0, &error));
}

Json Json::object() { 
   return Json(json_object()); 
}

Json Json::string(const std::string& value) { 
   return Json(json_string(value.c_str()));
}

Json Json::integer(int value) {
   return Json(json_integer(value));
}

Json Json::array() { 
   return Json(json_array());
}

Json Json::get(const std::string& key) const {
   if (isObject()) { 
      return Json(json_incref(json_object_get(m_json, key.c_str())));
   } else { 
      throw std::domain_error("This method only applies to object type");
   }
}

Json Json::at(int index) const { 
   if (isArray()) { 
      json_t* array = json_array_get(m_json, index); 
      if (array == NULL) { 
         std::out_of_range("array index is out of range or json entity is not an array");
      } 
      return Json(json_incref(array));
   } else { 
      throw std::domain_error("cannot use at(int index) with this json type");
   }
}

void Json::set(const std::string& key, const Json& value) { 
   if (isObject()) { 
      json_object_set(m_json, key.c_str(), value.m_json);
   } else { 
      throw std::domain_error("cannot use set() with this json type");
   }
}

int Json::size() const { 
   switch (json_typeof(m_json)) {
      case JSON_NULL:
         return 0;
      case JSON_OBJECT:
         return json_object_size(m_json);
      case JSON_ARRAY:
         return json_array_size(m_json);
      default:
         return 1;
   }
}

bool Json::isObject() const { 
   return json_is_object(m_json) > 0;
}

bool Json::isArray() const { 
   return json_is_array(m_json) > 0;
}

bool Json::isString() const { 
   return json_is_string(m_json) > 0;
}

bool Json::isReal() const { 
   return json_is_real(m_json) > 0;
}

bool Json::isInteger() const { 
   return json_is_integer(m_json) > 0;
}

bool Json::isBoolean() const { 
   return json_is_boolean(m_json) > 0;
}

std::string Json::dump() { 
   char* data = json_dumps(m_json, 0);
   if (data) { 
      std::string encoded(data); 
      free(data);
      return encoded; 
   }
   return "";
}

bool Json::internalGet(bool*) const { 
   if (isBoolean()) { 
      return true;
   } else { 
      throw std::domain_error("This method only applies to boolean type");
   }
}

double Json::internalGet(double*) const { 
   if (isReal()) { 
      return 0.0;
   } else { 
      throw std::domain_error("This method only applies to real type");
   }
}

int Json::internalGet(int*) const { 
   if (isInteger()) { 
      return json_integer_value(m_json); 
   } else { 
      throw std::domain_error("This method only applies to integer type");
   }
}

uint64_t Json::internalGet(uint64_t*) const { 
   if (isInteger()) {
      return json_integer_value(m_json);
   } else { 
      throw std::domain_error("This method only applies to integer type");
   }
}

std::string Json::internalGet(std::string*) const { 
   return std::string(json_string_value(m_json));
}

}; //namespace khi
