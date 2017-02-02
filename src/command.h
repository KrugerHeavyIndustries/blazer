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

#ifndef COMMAND_H
#define COMMAND_H

#include <cstddef>
#include <string>
#include <map>
#include <algorithm>

class CommandLine;

namespace khi {
class BB_Bucket;
class BB_Object; 
class BB;
}

namespace khi { 
namespace command { 

struct Base { 

   virtual bool valid(size_t wordc) = 0; 

   virtual int execute(size_t wordc, CommandLine& cmds, BB& aws) = 0;

   virtual void printUsage() = 0;

   virtual ~Base() {};

   protected: 
   
   void printBucket(const BB_Bucket& bucket, bool bucketName = false) const;

   void printObject(const BB_Object& object, bool longFormat = false) const;

   void parse1(int& idx, const CommandLine& cmds, std::string& position1); 

   void parse2(int& idx, const CommandLine& cmds, std::string& position1, std::string& position2);

   void parse3(int& idx, const CommandLine& cmds, std::string& position1, std::string& position2, std::string& position3);

   void parse4(int& idx, const CommandLine& cmds, std::string& position1, std::string& position2, std::string& position3, std::string& position4);

   int parseOrDefault(const std::string& number, int def) const;

   struct PrintObject : public std::unary_function<BB_Object, void> { 
      const Base& m_command;
      PrintObject(const Base& command) : m_command(command) {
      }
      void operator()(const BB_Object& file) const { 
         m_command.printObject(file);
      } 
   };

   friend struct PrintObject;
};

struct Dispatcher : public std::map<std::string, Base*> { 

   struct deleter
   {
      template <typename T>
      void operator()(const T& p) const {
         delete p.second;
      }
   };

   struct printer
   {
      template <typename T>
      void operator()(const T& p) const  {
         p.second->printUsage();
      }
   };

   template <class T>
   void add(const std::string& key) {
      insert(std::make_pair(key, new T()));
   }

   void printUsages() const {
      std::for_each(begin(), end(), printer());
   }

   ~Dispatcher() {
      std::for_each(begin(), end(), deleter());
   }
};

} // namespace command
} // namespace khi

#include "command_ls.h"
#include "command_upload_file.h"
#include "command_file_by_id.h" 
#include "command_file_by_name.h"
#include "command_list_file_versions.h"
#include "command_get_file_info.h"
#include "command_hide_file.h"
#include "command_delete_file_version.h"
#include "command_create_bucket.h"
#include "command_delete_bucket.h"
#include "command_update_bucket.h"
#include "command_list_buckets.h"

#endif // COMMAND_H

