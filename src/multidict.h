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

#ifndef MULTIDICT_H
#define MULTIDICT_H

#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <exception>
#include <stdexcept>

struct Dictionary_Error: public std::runtime_error {
	Dictionary_Error(const std::string & msg = ""): std::runtime_error(msg) {}
};

// A simple string-to-string dictionary, with additional methods for conversion to doubles and integers.
class MultiDict {
   std::multimap<std::string, std::string> entries;

   public:

   MultiDict() {}
   typedef std::multimap<std::string, std::string>::iterator iterator;
   typedef std::multimap<std::string, std::string>::const_iterator const_iterator;
    
   iterator begin() {return entries.begin();}
   const_iterator begin() const {return entries.begin();}
    
   iterator end() {return entries.end();}
   const_iterator end() const {return entries.end();}
    
   std::pair<iterator, iterator> equal_range(const std::string & key) {
       return entries.equal_range(key);
   }

   std::pair<const_iterator, const_iterator> equal_range(const std::string & key) const {
       return entries.equal_range(key);
   }
    
   void clear() { entries.clear(); }
    
   bool exists(const std::string & key) const { return (entries.find(key) != entries.end()); }
    
   // get first value for key if one exists. Return true if a value found for key,
   // return false otherwise.
   bool get(const std::string & key, std::string & value) const {
        const_iterator val = entries.find(key);
        if(val != entries.end()) value = val->second;
        return (val != entries.end());
   }
   bool get(const std::string & key, double & value) const {
       const_iterator val = entries.find(key);
       if(val != entries.end()) value = strtod(val->second.c_str(), NULL);
       return (val != entries.end());
   }
   bool get(const std::string & key, int & value) const {
       const_iterator val = entries.find(key);
       if(val != entries.end()) value = strtol(val->second.c_str(), NULL, 0);
       return (val != entries.end());
   }
   bool get(const std::string & key, long & value) const {
       const_iterator val = entries.find(key);
       if(val != entries.end()) value = strtol(val->second.c_str(), NULL, 0);
       return (val != entries.end());
   }
   bool get(const std::string & key, size_t & value) const {
       const_iterator val = entries.find(key);
       if(val != entries.end()) value = strtol(val->second.c_str(), NULL, 0);
       return (val != entries.end());
   }
    
    // get first value for key if one exists. Return value if found for key,
    // return defaultVal otherwise.
   const std::string & getWithDefault(const std::string & key, const std::string & defaultVal) const {
       const_iterator val = entries.find(key);
       return (val != entries.end())? val->second : defaultVal;
   }
   double getWithDefault(const std::string & key, double defaultVal) const {
       const_iterator val = entries.find(key);
       return (val != entries.end())? strtod(val->second.c_str(), NULL) : defaultVal;
   }
   int getWithDefault(const std::string & key, int defaultVal) const {
       const_iterator val = entries.find(key);
       return (val != entries.end())? strtol(val->second.c_str(), NULL, 0) : defaultVal;
   }
   long getWithDefault(const std::string & key, long defaultVal) const {
       const_iterator val = entries.find(key);
       return (val != entries.end())? strtol(val->second.c_str(), NULL, 0) : defaultVal;
   }
   size_t getWithDefault(const std::string & key, size_t defaultVal) const {
       const_iterator val = entries.find(key);
       return (val != entries.end())? strtol(val->second.c_str(), NULL, 0) : defaultVal;
   }
    
   // insert entry into dictionary, regardless of existence of previous entries with key.
   void insert(const std::string & key, const std::string & value) {
       entries.insert(std::make_pair(key, value));
   }
    
   // set value for existing key if possible, insert entry into dictionary if no value for key
   // exists.
   void set(const std::string & key, const std::string & value) {
       iterator val = entries.find(key);
       if(val == entries.end())
           insert(key, value);
       else
           val->second = value;
   }
};

#endif // MULTIDICT_H
