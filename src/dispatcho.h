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

#ifndef DISPATCHO_H
#define DISPATCHO_H

#include <deque>
#include <string>
#include <pthread.h>

namespace khi {

class Task { 
   
public: 

   Task(void* arg = NULL, const std::string name = "")
      : m_arg(arg),
        m_name(name) {
   }

   virtual ~Task() {
   }

   void setArg(void* arg) { 
      m_arg = arg; 
   }

   virtual int run() = 0;

protected: 
   
   void* m_arg; 
   std::string m_name;

};

class Dispatcho {

public:

   Dispatcho(int numThreads = 10); 
   ~Dispatcho(); 

   int async(Task* task); 

   int stop();

   int size();

private:

   Dispatcho& operator=(const Dispatcho&); // prevent assign
   Dispatcho(const Dispatcho&); // prevent copy

   int createThreads();

   Task* take();

   static void* threadMain(void* threadData);

   volatile bool m_running;
   int m_numThreads; 
   std::deque<Task*> m_queue;

   int* m_results;
   pthread_t* m_threads;
   pthread_mutex_t m_mutex;
   pthread_cond_t  m_condition;
};

}
#endif // DISPATCHO_H
