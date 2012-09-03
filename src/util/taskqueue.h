//
// BAGEL - Parallel electron correlation program.
// Filename: taskqueue.h
// Copyright (C) 2012 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and\/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// The BAGEL package is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the BAGEL package; see COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//


#ifndef __SRC_UTIL_TASKQUEUE_H
#define __SRC_UTIL_TASKQUEUE_H

#include <cstddef>
#include <list>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <boost/thread/thread.hpp>

namespace bagel {

template<typename T>
class TaskQueue {
  protected:
    std::vector<T> task_;
    std::list<std::shared_ptr<std::atomic_flag> > flag_;
    static const int chunck_ = 12;

  public:
    TaskQueue(std::vector<T>& t) : task_(t) {}

    void compute(const int num_threads) {
#ifndef _OPENMP
      for (int i = 0; i != (task_.size()-1)/chunck_+1; ++i)
        flag_.push_back(std::shared_ptr<std::atomic_flag>(new std::atomic_flag(ATOMIC_FLAG_INIT)));
      std::list<std::shared_ptr<boost::thread> > threads;
      for (int i = 0; i != num_threads; ++i)
        threads.push_back(std::shared_ptr<boost::thread>(new boost::thread(boost::bind(&TaskQueue<T>::compute_one_thread, this))));
      for (auto i = threads.begin(); i != threads.end(); ++i)
        (*i)->join();
#else
      const size_t n = task_.size();
      #pragma omp parallel for schedule(dynamic,chunck_)
      for (size_t i = 0; i < n; ++i)
        task_[i].compute();
#endif
    }

    void compute_one_thread() {
      int j = 0;
      for (auto i = flag_.begin(); i != flag_.end(); ++i, j += chunck_)
        if (!(*i)->test_and_set()) {
          task_[j].compute();
          for (int k = 1; k < chunck_; ++k)
            if (j+k < task_.size()) task_[j+k].compute();
        }
    }
};

}

#endif
