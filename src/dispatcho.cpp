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

#include "dispatcho.h"

#include <cstdlib>
#include <cassert>

#include <algorithm>

namespace khi {

struct AllExitSuccess
{
   bool operator() (const int c) const { return c == EXIT_SUCCESS; }
};

Dispatcho::Dispatcho(int numThreads) { 
   m_numThreads = numThreads; 
   m_results = new int[m_numThreads]();
   createThreads();
   m_running = true;
   m_drain = false;
}

Dispatcho::~Dispatcho() { 
   stop(); 
}

int Dispatcho::createThreads() {
   pthread_mutex_init(&m_mutex, NULL);
   pthread_cond_init(&m_condition, NULL);
   m_threads = new pthread_t[m_numThreads];
   for (int i = 0; i < m_numThreads; ++i) { 
      pthread_create(&m_threads[i], NULL, threadMain, this);
   }
   return 0;
}

int Dispatcho::async(Task* task) { 
   pthread_mutex_lock(&m_mutex);
   m_queue.push_back(task);
   int size = m_queue.size();
   pthread_mutex_unlock(&m_mutex);
   pthread_cond_signal(&m_condition);
   return size; 
}

int Dispatcho::workoff() {
   int ret = EXIT_SUCCESS;
   if (!m_running) { 
      return EXIT_FAILURE;
   }
   m_drain = true;
   pthread_cond_broadcast(&m_condition);

   for (int i = 0; i < m_numThreads; i++) {
      pthread_join(m_threads[i], reinterpret_cast<void**>(&m_results[i]));
   }

   ret = std::all_of(m_results, m_results + m_numThreads, AllExitSuccess()) ? EXIT_SUCCESS : EXIT_FAILURE;

   return ret;
}

int Dispatcho::stop() {
   int ret = EXIT_SUCCESS;
   if (!m_running) {
      return EXIT_FAILURE;
   }
   m_running = false;
   pthread_cond_broadcast(&m_condition);

   ret = std::all_of(m_results, m_results + m_numThreads, AllExitSuccess()) ? EXIT_SUCCESS : EXIT_FAILURE;

   delete [] m_results;
   delete [] m_threads;
   pthread_mutex_destroy(&m_mutex);
   pthread_cond_destroy(&m_condition);

   return ret;
}

int Dispatcho::size() {
   pthread_mutex_lock(&m_mutex);
   int size = m_queue.size();
   pthread_mutex_unlock(&m_mutex);
   return size;
}

Task* Dispatcho::take() {
   Task* task = NULL;
   if (m_queue.empty() && m_drain)
      return task;
   if (m_running) {
      while (m_queue.empty()) {
          pthread_mutex_lock(&m_mutex);
          pthread_cond_wait(&m_condition, &m_mutex);
          pthread_mutex_unlock(&m_mutex);
      }
      assert(!m_queue.empty());
      pthread_mutex_lock(&m_mutex);
      task = m_queue.front();
      m_queue.pop_front();
      pthread_mutex_unlock(&m_mutex);
   }
   return task;
}

void* Dispatcho::threadMain(void* arg) {
  int ret = EXIT_SUCCESS;
  pthread_t tid = pthread_self();
  Dispatcho* dispatcho = static_cast<Dispatcho*>(arg);
  while (dispatcho->m_running && ret == EXIT_SUCCESS) {
      Task* task = dispatcho->take();
      if (task == NULL) {
          break;
      }
      assert(task);
      try {
         ret = task->run();
      } catch (...) {
         ret = EXIT_FAILURE;
      }
  }
  pthread_exit(reinterpret_cast<void*>(ret));
}

} // namespace khi
