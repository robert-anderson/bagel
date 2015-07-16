//
// BAGEL - Parallel electron correlation program.
// Filename: RelMRCI_tasks13.cc
// Copyright (C) 2014 Shiozaki group
//
// Author: Shiozaki group <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
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

#include <bagel_config.h>
#ifdef COMPILE_SMITH

#include <src/smith/RelMRCI_tasks13.h>

using namespace std;
using namespace bagel;
using namespace bagel::SMITH;
using namespace bagel::SMITH::RelMRCI;

void Task600::Task_local::compute() {
  const Index c4 = b(0);
  const Index a3 = b(1);
  const Index x1 = b(2);
  const Index x0 = b(3);
  // tensor label: I1017
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c4, a3, x1, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c4, a3, x1, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c4, a3, x1, x0), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      // tensor label: Gamma24
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x2, x1, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x2, x1, x0)]);
      sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x2.size(), x1.size(), x0.size());
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c4, a3, x3, x2);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c4, a3, x3, x2)]);
      sort_indices<2,3,0,1,0,1,-1,1>(i1data, i1data_sorted, c4.size(), a3.size(), x3.size(), x2.size());
      zgemm3m_("T", "N", x1.size()*x0.size(), c4.size()*a3.size(), x3.size()*x2.size(),
             1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
             1.0, odata_sorted, x1.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x1.size(), x0.size(), c4.size(), a3.size());
  out()->put_block(odata, c4, a3, x1, x0);
}

void Task601::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x1 : *range_[1]) {
    for (auto& c4 : *range_[0]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x1, a3, c2, c4);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x1, a3, c2, c4)]);
      sort_indices<0,3,1,2,0,1,1,1>(i0data, i0data_sorted, x1.size(), a3.size(), c2.size(), c4.size());
      // tensor label: I1020
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c4, a1, x1, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c4, a1, x1, x0)]);
      sort_indices<2,0,1,3,0,1,1,1>(i1data, i1data_sorted, c4.size(), a1.size(), x1.size(), x0.size());
      zgemm3m_("T", "N", a3.size()*c2.size(), a1.size()*x0.size(), c4.size()*x1.size(),
             1.0, i0data_sorted, c4.size()*x1.size(), i1data_sorted, c4.size()*x1.size(),
             1.0, odata_sorted, a3.size()*c2.size());
    }
  }
  sort_indices<1,0,3,2,1,1,1,1>(odata_sorted, odata, a3.size(), c2.size(), a1.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task602::Task_local::compute() {
  const Index c4 = b(0);
  const Index a1 = b(1);
  const Index x1 = b(2);
  const Index x0 = b(3);
  // tensor label: I1020
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c4, a1, x1, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c4, a1, x1, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c4, a1, x1, x0), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      // tensor label: Gamma24
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x2, x1, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x2, x1, x0)]);
      sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x2.size(), x1.size(), x0.size());
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c4, a1, x3, x2);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c4, a1, x3, x2)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, c4.size(), a1.size(), x3.size(), x2.size());
      zgemm3m_("T", "N", x1.size()*x0.size(), c4.size()*a1.size(), x3.size()*x2.size(),
             1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
             1.0, odata_sorted, x1.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x1.size(), x0.size(), c4.size(), a1.size());
  out()->put_block(odata, c4, a1, x1, x0);
}

void Task603::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x1 : *range_[1]) {
    for (auto& a4 : *range_[2]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x1, a3, a4, a1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x1, a3, a4, a1)]);
      sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x1.size(), a3.size(), a4.size(), a1.size());
      // tensor label: I1023
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, a4, x1, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, a4, x1, x0)]);
      sort_indices<2,1,0,3,0,1,1,1>(i1data, i1data_sorted, c2.size(), a4.size(), x1.size(), x0.size());
      zgemm3m_("T", "N", a3.size()*a1.size(), c2.size()*x0.size(), a4.size()*x1.size(),
             1.0, i0data_sorted, a4.size()*x1.size(), i1data_sorted, a4.size()*x1.size(),
             1.0, odata_sorted, a3.size()*a1.size());
    }
  }
  sort_indices<2,0,3,1,1,1,1,1>(odata_sorted, odata, a3.size(), a1.size(), c2.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task604::Task_local::compute() {
  const Index c2 = b(0);
  const Index a4 = b(1);
  const Index x1 = b(2);
  const Index x0 = b(3);
  // tensor label: I1023
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a4, x1, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a4, x1, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a4, x1, x0), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      // tensor label: Gamma24
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x2, x1, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x2, x1, x0)]);
      sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x2.size(), x1.size(), x0.size());
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, a4, x3, x2);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, a4, x3, x2)]);
      sort_indices<2,3,0,1,0,1,-1,1>(i1data, i1data_sorted, c2.size(), a4.size(), x3.size(), x2.size());
      zgemm3m_("T", "N", x1.size()*x0.size(), c2.size()*a4.size(), x3.size()*x2.size(),
             1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
             1.0, odata_sorted, x1.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x1.size(), x0.size(), c2.size(), a4.size());
  out()->put_block(odata, c2, a4, x1, x0);
}

void Task605::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x1 : *range_[1]) {
    for (auto& a4 : *range_[2]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x1, a1, a4, a3);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x1, a1, a4, a3)]);
      sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x1.size(), a1.size(), a4.size(), a3.size());
      // tensor label: I1026
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, a4, x1, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, a4, x1, x0)]);
      sort_indices<2,1,0,3,0,1,1,1>(i1data, i1data_sorted, c2.size(), a4.size(), x1.size(), x0.size());
      zgemm3m_("T", "N", a1.size()*a3.size(), c2.size()*x0.size(), a4.size()*x1.size(),
             1.0, i0data_sorted, a4.size()*x1.size(), i1data_sorted, a4.size()*x1.size(),
             1.0, odata_sorted, a1.size()*a3.size());
    }
  }
  sort_indices<2,1,3,0,1,1,1,1>(odata_sorted, odata, a1.size(), a3.size(), c2.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task606::Task_local::compute() {
  const Index c2 = b(0);
  const Index a4 = b(1);
  const Index x1 = b(2);
  const Index x0 = b(3);
  // tensor label: I1026
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a4, x1, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a4, x1, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a4, x1, x0), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      // tensor label: Gamma24
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x2, x1, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x2, x1, x0)]);
      sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x2.size(), x1.size(), x0.size());
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, a4, x3, x2);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, a4, x3, x2)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, c2.size(), a4.size(), x3.size(), x2.size());
      zgemm3m_("T", "N", x1.size()*x0.size(), c2.size()*a4.size(), x3.size()*x2.size(),
             1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
             1.0, odata_sorted, x1.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x1.size(), x0.size(), c2.size(), a4.size());
  out()->put_block(odata, c2, a4, x1, x0);
}

void Task607::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c2, a1, x2, x1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(c2, a1, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i0data, i0data_sorted, c2.size(), a1.size(), x2.size(), x1.size());
      // tensor label: I1029
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a3, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a3, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, a3.size(), x0.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", c2.size()*a1.size(), a3.size()*x0.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, c2.size()*a1.size());
    }
  }
  sort_indices<0,2,3,1,1,1,1,1>(odata_sorted, odata, c2.size(), a1.size(), a3.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task608::Task_local::compute() {
  const Index a3 = b(0);
  const Index x0 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1029
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a3, x0, x2, x1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a3, x0, x2, x1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a3, x0, x2, x1), 0.0);
  for (auto& x5 : *range_[1]) {
    for (auto& x4 : *range_[1]) {
      for (auto& x3 : *range_[1]) {
        // tensor label: Gamma32
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x5, x0, x4, x3, x2, x1);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x5, x0, x4, x3, x2, x1)]);
        sort_indices<0,2,3,1,4,5,0,1,1,1>(i0data, i0data_sorted, x5.size(), x0.size(), x4.size(), x3.size(), x2.size(), x1.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x5, a3, x4, x3);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x5, a3, x4, x3)]);
        sort_indices<0,2,3,1,0,1,-1,2>(i1data, i1data_sorted, x5.size(), a3.size(), x4.size(), x3.size());
        zgemm3m_("T", "N", x0.size()*x2.size()*x1.size(), a3.size(), x5.size()*x4.size()*x3.size(),
               1.0, i0data_sorted, x5.size()*x4.size()*x3.size(), i1data_sorted, x5.size()*x4.size()*x3.size(),
               1.0, odata_sorted, x0.size()*x2.size()*x1.size());
      }
    }
  }
  sort_indices<3,0,1,2,1,1,1,1>(odata_sorted, odata, x0.size(), x2.size(), x1.size(), a3.size());
  out()->put_block(odata, a3, x0, x2, x1);
}

void Task609::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c2, a3, x2, x1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(c2, a3, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i0data, i0data_sorted, c2.size(), a3.size(), x2.size(), x1.size());
      // tensor label: I1032
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a1, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a1, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, a1.size(), x0.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", c2.size()*a3.size(), a1.size()*x0.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, c2.size()*a3.size());
    }
  }
  sort_indices<0,1,3,2,1,1,1,1>(odata_sorted, odata, c2.size(), a3.size(), a1.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task610::Task_local::compute() {
  const Index a1 = b(0);
  const Index x0 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1032
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a1, x0, x2, x1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a1, x0, x2, x1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a1, x0, x2, x1), 0.0);
  for (auto& x5 : *range_[1]) {
    for (auto& x4 : *range_[1]) {
      for (auto& x3 : *range_[1]) {
        // tensor label: Gamma32
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x5, x0, x4, x3, x2, x1);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x5, x0, x4, x3, x2, x1)]);
        sort_indices<0,2,3,1,4,5,0,1,1,1>(i0data, i0data_sorted, x5.size(), x0.size(), x4.size(), x3.size(), x2.size(), x1.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x5, a1, x4, x3);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x5, a1, x4, x3)]);
        sort_indices<0,2,3,1,0,1,1,2>(i1data, i1data_sorted, x5.size(), a1.size(), x4.size(), x3.size());
        zgemm3m_("T", "N", x0.size()*x2.size()*x1.size(), a1.size(), x5.size()*x4.size()*x3.size(),
               1.0, i0data_sorted, x5.size()*x4.size()*x3.size(), i1data_sorted, x5.size()*x4.size()*x3.size(),
               1.0, odata_sorted, x0.size()*x2.size()*x1.size());
      }
    }
  }
  sort_indices<3,0,1,2,1,1,1,1>(odata_sorted, odata, x0.size(), x2.size(), x1.size(), a1.size());
  out()->put_block(odata, a1, x0, x2, x1);
}

void Task611::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c2, x2, x1, a1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(c2, x2, x1, a1)]);
      sort_indices<1,2,0,3,0,1,1,1>(i0data, i0data_sorted, c2.size(), x2.size(), x1.size(), a1.size());
      // tensor label: I1035
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a3, x2, x1, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a3, x2, x1, x0)]);
      sort_indices<1,2,0,3,0,1,1,1>(i1data, i1data_sorted, a3.size(), x2.size(), x1.size(), x0.size());
      zgemm3m_("T", "N", c2.size()*a1.size(), a3.size()*x0.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, c2.size()*a1.size());
    }
  }
  sort_indices<0,2,3,1,1,1,1,1>(odata_sorted, odata, c2.size(), a1.size(), a3.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task612::Task_local::compute() {
  const Index a3 = b(0);
  const Index x2 = b(1);
  const Index x1 = b(2);
  const Index x0 = b(3);
  // tensor label: I1035
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a3, x2, x1, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a3, x2, x1, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a3, x2, x1, x0), 0.0);
  for (auto& x5 : *range_[1]) {
    for (auto& x4 : *range_[1]) {
      for (auto& x3 : *range_[1]) {
        // tensor label: Gamma26
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x5, x2, x4, x3, x1, x0);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x5, x2, x4, x3, x1, x0)]);
        sort_indices<0,2,3,1,4,5,0,1,1,1>(i0data, i0data_sorted, x5.size(), x2.size(), x4.size(), x3.size(), x1.size(), x0.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x5, a3, x4, x3);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x5, a3, x4, x3)]);
        sort_indices<0,2,3,1,0,1,-1,2>(i1data, i1data_sorted, x5.size(), a3.size(), x4.size(), x3.size());
        zgemm3m_("T", "N", x2.size()*x1.size()*x0.size(), a3.size(), x5.size()*x4.size()*x3.size(),
               1.0, i0data_sorted, x5.size()*x4.size()*x3.size(), i1data_sorted, x5.size()*x4.size()*x3.size(),
               1.0, odata_sorted, x2.size()*x1.size()*x0.size());
      }
    }
  }
  sort_indices<3,0,1,2,1,1,1,1>(odata_sorted, odata, x2.size(), x1.size(), x0.size(), a3.size());
  out()->put_block(odata, a3, x2, x1, x0);
}

void Task613::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c2, x2, x1, a3);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(c2, x2, x1, a3)]);
      sort_indices<1,2,0,3,0,1,1,1>(i0data, i0data_sorted, c2.size(), x2.size(), x1.size(), a3.size());
      // tensor label: I1038
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a1, x0, x1, x2);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a1, x0, x1, x2)]);
      sort_indices<3,2,0,1,0,1,1,1>(i1data, i1data_sorted, a1.size(), x0.size(), x1.size(), x2.size());
      zgemm3m_("T", "N", c2.size()*a3.size(), a1.size()*x0.size(), x1.size()*x2.size(),
             1.0, i0data_sorted, x1.size()*x2.size(), i1data_sorted, x1.size()*x2.size(),
             1.0, odata_sorted, c2.size()*a3.size());
    }
  }
  sort_indices<0,1,3,2,1,1,1,1>(odata_sorted, odata, c2.size(), a3.size(), a1.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task614::Task_local::compute() {
  const Index a1 = b(0);
  const Index x0 = b(1);
  const Index x1 = b(2);
  const Index x2 = b(3);
  // tensor label: I1038
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a1, x0, x1, x2);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a1, x0, x1, x2)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a1, x0, x1, x2), 0.0);
  for (auto& x5 : *range_[1]) {
    for (auto& x4 : *range_[1]) {
      for (auto& x3 : *range_[1]) {
        // tensor label: Gamma335
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x5, x0, x4, x3, x1, x2);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x5, x0, x4, x3, x1, x2)]);
        sort_indices<0,2,3,1,4,5,0,1,1,1>(i0data, i0data_sorted, x5.size(), x0.size(), x4.size(), x3.size(), x1.size(), x2.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x5, a1, x4, x3);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x5, a1, x4, x3)]);
        sort_indices<0,2,3,1,0,1,1,2>(i1data, i1data_sorted, x5.size(), a1.size(), x4.size(), x3.size());
        zgemm3m_("T", "N", x0.size()*x1.size()*x2.size(), a1.size(), x5.size()*x4.size()*x3.size(),
               1.0, i0data_sorted, x5.size()*x4.size()*x3.size(), i1data_sorted, x5.size()*x4.size()*x3.size(),
               1.0, odata_sorted, x0.size()*x1.size()*x2.size());
      }
    }
  }
  sort_indices<3,0,1,2,1,1,1,1>(odata_sorted, odata, x0.size(), x1.size(), x2.size(), a1.size());
  out()->put_block(odata, a1, x0, x1, x2);
}

void Task615::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x2, a1, c2, x1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x2, a1, c2, x1)]);
      sort_indices<0,3,1,2,0,1,1,1>(i0data, i0data_sorted, x2.size(), a1.size(), c2.size(), x1.size());
      // tensor label: I1041
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a3, x1, x2, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a3, x1, x2, x0)]);
      sort_indices<2,1,0,3,0,1,1,1>(i1data, i1data_sorted, a3.size(), x1.size(), x2.size(), x0.size());
      zgemm3m_("T", "N", a1.size()*c2.size(), a3.size()*x0.size(), x1.size()*x2.size(),
             1.0, i0data_sorted, x1.size()*x2.size(), i1data_sorted, x1.size()*x2.size(),
             1.0, odata_sorted, a1.size()*c2.size());
    }
  }
  sort_indices<1,2,3,0,1,1,1,1>(odata_sorted, odata, a1.size(), c2.size(), a3.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task616::Task_local::compute() {
  const Index a3 = b(0);
  const Index x1 = b(1);
  const Index x2 = b(2);
  const Index x0 = b(3);
  // tensor label: I1041
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a3, x1, x2, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a3, x1, x2, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a3, x1, x2, x0), 0.0);
  for (auto& x5 : *range_[1]) {
    for (auto& x4 : *range_[1]) {
      for (auto& x3 : *range_[1]) {
        // tensor label: Gamma336
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x5, x1, x4, x3, x2, x0);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x5, x1, x4, x3, x2, x0)]);
        sort_indices<0,2,3,1,4,5,0,1,1,1>(i0data, i0data_sorted, x5.size(), x1.size(), x4.size(), x3.size(), x2.size(), x0.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x5, a3, x4, x3);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x5, a3, x4, x3)]);
        sort_indices<0,2,3,1,0,1,1,2>(i1data, i1data_sorted, x5.size(), a3.size(), x4.size(), x3.size());
        zgemm3m_("T", "N", x1.size()*x2.size()*x0.size(), a3.size(), x5.size()*x4.size()*x3.size(),
               1.0, i0data_sorted, x5.size()*x4.size()*x3.size(), i1data_sorted, x5.size()*x4.size()*x3.size(),
               1.0, odata_sorted, x1.size()*x2.size()*x0.size());
      }
    }
  }
  sort_indices<3,0,1,2,1,1,1,1>(odata_sorted, odata, x1.size(), x2.size(), x0.size(), a3.size());
  out()->put_block(odata, a3, x1, x2, x0);
}

void Task617::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x2, a3, c2, x1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x2, a3, c2, x1)]);
      sort_indices<0,3,1,2,0,1,1,1>(i0data, i0data_sorted, x2.size(), a3.size(), c2.size(), x1.size());
      // tensor label: I1044
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a1, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a1, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, a1.size(), x0.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", a3.size()*c2.size(), a1.size()*x0.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, a3.size()*c2.size());
    }
  }
  sort_indices<1,0,3,2,1,1,1,1>(odata_sorted, odata, a3.size(), c2.size(), a1.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task618::Task_local::compute() {
  const Index a1 = b(0);
  const Index x0 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1044
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a1, x0, x2, x1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a1, x0, x2, x1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a1, x0, x2, x1), 0.0);
  for (auto& x5 : *range_[1]) {
    for (auto& x4 : *range_[1]) {
      for (auto& x3 : *range_[1]) {
        // tensor label: Gamma32
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x5, x0, x4, x3, x2, x1);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x5, x0, x4, x3, x2, x1)]);
        sort_indices<0,2,3,1,4,5,0,1,1,1>(i0data, i0data_sorted, x5.size(), x0.size(), x4.size(), x3.size(), x2.size(), x1.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x5, a1, x4, x3);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x5, a1, x4, x3)]);
        sort_indices<0,2,3,1,0,1,-1,2>(i1data, i1data_sorted, x5.size(), a1.size(), x4.size(), x3.size());
        zgemm3m_("T", "N", x0.size()*x2.size()*x1.size(), a1.size(), x5.size()*x4.size()*x3.size(),
               1.0, i0data_sorted, x5.size()*x4.size()*x3.size(), i1data_sorted, x5.size()*x4.size()*x3.size(),
               1.0, odata_sorted, x0.size()*x2.size()*x1.size());
      }
    }
  }
  sort_indices<3,0,1,2,1,1,1,1>(odata_sorted, odata, x0.size(), x2.size(), x1.size(), a1.size());
  out()->put_block(odata, a1, x0, x2, x1);
}

void Task619::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x2, x1, c2, a1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x2, x1, c2, a1)]);
      sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x2.size(), x1.size(), c2.size(), a1.size());
      // tensor label: I1047
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a3, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a3, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, a3.size(), x0.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", c2.size()*a1.size(), a3.size()*x0.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, c2.size()*a1.size());
    }
  }
  sort_indices<0,2,3,1,1,1,1,1>(odata_sorted, odata, c2.size(), a1.size(), a3.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task620::Task_local::compute() {
  const Index a3 = b(0);
  const Index x0 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1047
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a3, x0, x2, x1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a3, x0, x2, x1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a3, x0, x2, x1), 0.0);
  for (auto& x5 : *range_[1]) {
    for (auto& x4 : *range_[1]) {
      for (auto& x3 : *range_[1]) {
        // tensor label: Gamma32
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x5, x0, x4, x3, x2, x1);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x5, x0, x4, x3, x2, x1)]);
        sort_indices<0,2,3,1,4,5,0,1,1,1>(i0data, i0data_sorted, x5.size(), x0.size(), x4.size(), x3.size(), x2.size(), x1.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x5, a3, x4, x3);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x5, a3, x4, x3)]);
        sort_indices<0,2,3,1,0,1,-1,2>(i1data, i1data_sorted, x5.size(), a3.size(), x4.size(), x3.size());
        zgemm3m_("T", "N", x0.size()*x2.size()*x1.size(), a3.size(), x5.size()*x4.size()*x3.size(),
               1.0, i0data_sorted, x5.size()*x4.size()*x3.size(), i1data_sorted, x5.size()*x4.size()*x3.size(),
               1.0, odata_sorted, x0.size()*x2.size()*x1.size());
      }
    }
  }
  sort_indices<3,0,1,2,1,1,1,1>(odata_sorted, odata, x0.size(), x2.size(), x1.size(), a3.size());
  out()->put_block(odata, a3, x0, x2, x1);
}

void Task621::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x2, x1, c2, a3);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x2, x1, c2, a3)]);
      sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x2.size(), x1.size(), c2.size(), a3.size());
      // tensor label: I1050
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a1, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a1, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, a1.size(), x0.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", c2.size()*a3.size(), a1.size()*x0.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, c2.size()*a3.size());
    }
  }
  sort_indices<0,1,3,2,1,1,1,1>(odata_sorted, odata, c2.size(), a3.size(), a1.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task622::Task_local::compute() {
  const Index a1 = b(0);
  const Index x0 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1050
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a1, x0, x2, x1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a1, x0, x2, x1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a1, x0, x2, x1), 0.0);
  for (auto& x5 : *range_[1]) {
    for (auto& x4 : *range_[1]) {
      for (auto& x3 : *range_[1]) {
        // tensor label: Gamma32
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x5, x0, x4, x3, x2, x1);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x5, x0, x4, x3, x2, x1)]);
        sort_indices<0,2,3,1,4,5,0,1,1,1>(i0data, i0data_sorted, x5.size(), x0.size(), x4.size(), x3.size(), x2.size(), x1.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x5, a1, x4, x3);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x5, a1, x4, x3)]);
        sort_indices<0,2,3,1,0,1,1,2>(i1data, i1data_sorted, x5.size(), a1.size(), x4.size(), x3.size());
        zgemm3m_("T", "N", x0.size()*x2.size()*x1.size(), a1.size(), x5.size()*x4.size()*x3.size(),
               1.0, i0data_sorted, x5.size()*x4.size()*x3.size(), i1data_sorted, x5.size()*x4.size()*x3.size(),
               1.0, odata_sorted, x0.size()*x2.size()*x1.size());
      }
    }
  }
  sort_indices<3,0,1,2,1,1,1,1>(odata_sorted, odata, x0.size(), x2.size(), x1.size(), a1.size());
  out()->put_block(odata, a1, x0, x2, x1);
}

void Task623::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& a4 : *range_[2]) {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c2, a3, a4, a1);
    std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(c2, a3, a4, a1)]);
    sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, c2.size(), a3.size(), a4.size(), a1.size());
    // tensor label: I1053
    std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a4, x0);
    std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a4, x0)]);
    sort_indices<0,1,0,1,1,1>(i1data, i1data_sorted, a4.size(), x0.size());
    zgemm3m_("T", "N", c2.size()*a3.size()*a1.size(), x0.size(), a4.size(),
           1.0, i0data_sorted, a4.size(), i1data_sorted, a4.size(),
           1.0, odata_sorted, c2.size()*a3.size()*a1.size());
  }
  sort_indices<0,1,3,2,1,1,1,1>(odata_sorted, odata, c2.size(), a3.size(), a1.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task624::Task_local::compute() {
  const Index a4 = b(0);
  const Index x0 = b(1);
  // tensor label: I1053
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a4, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a4, x0), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      for (auto& x1 : *range_[1]) {
        // tensor label: Gamma33
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x0, x2, x1);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x0, x2, x1)]);
        sort_indices<0,2,3,1,0,1,1,1>(i0data, i0data_sorted, x3.size(), x0.size(), x2.size(), x1.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x3, a4, x2, x1);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x3, a4, x2, x1)]);
        sort_indices<0,2,3,1,0,1,1,1>(i1data, i1data_sorted, x3.size(), a4.size(), x2.size(), x1.size());
        zgemm3m_("T", "N", x0.size(), a4.size(), x3.size()*x2.size()*x1.size(),
               1.0, i0data_sorted, x3.size()*x2.size()*x1.size(), i1data_sorted, x3.size()*x2.size()*x1.size(),
               1.0, odata_sorted, x0.size());
      }
    }
  }
  sort_indices<1,0,1,1,1,1>(odata_sorted, odata, x0.size(), a4.size());
  out()->put_block(odata, a4, x0);
}

void Task625::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& a4 : *range_[2]) {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c2, a1, a4, a3);
    std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(c2, a1, a4, a3)]);
    sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, c2.size(), a1.size(), a4.size(), a3.size());
    // tensor label: I1056
    std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a4, x0);
    std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a4, x0)]);
    sort_indices<0,1,0,1,1,1>(i1data, i1data_sorted, a4.size(), x0.size());
    zgemm3m_("T", "N", c2.size()*a1.size()*a3.size(), x0.size(), a4.size(),
           1.0, i0data_sorted, a4.size(), i1data_sorted, a4.size(),
           1.0, odata_sorted, c2.size()*a1.size()*a3.size());
  }
  sort_indices<0,2,3,1,1,1,1,1>(odata_sorted, odata, c2.size(), a1.size(), a3.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task626::Task_local::compute() {
  const Index a4 = b(0);
  const Index x0 = b(1);
  // tensor label: I1056
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a4, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a4, x0), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      for (auto& x1 : *range_[1]) {
        // tensor label: Gamma33
        std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x0, x2, x1);
        std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x0, x2, x1)]);
        sort_indices<0,2,3,1,0,1,1,1>(i0data, i0data_sorted, x3.size(), x0.size(), x2.size(), x1.size());
        // tensor label: t2
        std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x3, a4, x2, x1);
        std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x3, a4, x2, x1)]);
        sort_indices<0,2,3,1,0,1,-1,1>(i1data, i1data_sorted, x3.size(), a4.size(), x2.size(), x1.size());
        zgemm3m_("T", "N", x0.size(), a4.size(), x3.size()*x2.size()*x1.size(),
               1.0, i0data_sorted, x3.size()*x2.size()*x1.size(), i1data_sorted, x3.size()*x2.size()*x1.size(),
               1.0, odata_sorted, x0.size());
      }
    }
  }
  sort_indices<1,0,1,1,1,1>(odata_sorted, odata, x0.size(), a4.size());
  out()->put_block(odata, a4, x0);
}

void Task627::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& c4 : *range_[0]) {
    for (auto& c5 : *range_[0]) {
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c4, a3, c5, a1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(c4, a3, c5, a1)]);
      sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, c4.size(), a3.size(), c5.size(), a1.size());
      // tensor label: I1071
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c5, c2, c4, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c5, c2, c4, x0)]);
      sort_indices<2,0,1,3,0,1,1,1>(i1data, i1data_sorted, c5.size(), c2.size(), c4.size(), x0.size());
      zgemm3m_("T", "N", a3.size()*a1.size(), c2.size()*x0.size(), c5.size()*c4.size(),
             1.0, i0data_sorted, c5.size()*c4.size(), i1data_sorted, c5.size()*c4.size(),
             1.0, odata_sorted, a3.size()*a1.size());
    }
  }
  sort_indices<2,0,3,1,1,1,1,1>(odata_sorted, odata, a3.size(), a1.size(), c2.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task628::Task_local::compute() {
  const Index c5 = b(0);
  const Index c2 = b(1);
  const Index c4 = b(2);
  const Index x0 = b(3);
  // tensor label: I1071
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c5, c2, c4, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c5, c2, c4, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c5, c2, c4, x0), 0.0);
  for (auto& x1 : *range_[1]) {
    // tensor label: Gamma27
    std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x1, x0);
    std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x1, x0)]);
    sort_indices<0,1,0,1,1,1>(i0data, i0data_sorted, x1.size(), x0.size());
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x1, c5, c2, c4);
    std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x1, c5, c2, c4)]);
    sort_indices<0,1,2,3,0,1,2,1>(i1data, i1data_sorted, x1.size(), c5.size(), c2.size(), c4.size());
    zgemm3m_("T", "N", x0.size(), c5.size()*c2.size()*c4.size(), x1.size(),
           1.0, i0data_sorted, x1.size(), i1data_sorted, x1.size(),
           1.0, odata_sorted, x0.size());
  }
  sort_indices<1,2,3,0,1,1,1,1>(odata_sorted, odata, x0.size(), c5.size(), c2.size(), c4.size());
  out()->put_block(odata, c5, c2, c4, x0);
}

void Task629::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& c4 : *range_[0]) {
    for (auto& c5 : *range_[0]) {
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c4, a1, c5, a3);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(c4, a1, c5, a3)]);
      sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, c4.size(), a1.size(), c5.size(), a3.size());
      // tensor label: I1074
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c5, c2, c4, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c5, c2, c4, x0)]);
      sort_indices<2,0,1,3,0,1,1,1>(i1data, i1data_sorted, c5.size(), c2.size(), c4.size(), x0.size());
      zgemm3m_("T", "N", a1.size()*a3.size(), c2.size()*x0.size(), c5.size()*c4.size(),
             1.0, i0data_sorted, c5.size()*c4.size(), i1data_sorted, c5.size()*c4.size(),
             1.0, odata_sorted, a1.size()*a3.size());
    }
  }
  sort_indices<2,1,3,0,1,1,1,1>(odata_sorted, odata, a1.size(), a3.size(), c2.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task630::Task_local::compute() {
  const Index c5 = b(0);
  const Index c2 = b(1);
  const Index c4 = b(2);
  const Index x0 = b(3);
  // tensor label: I1074
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c5, c2, c4, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c5, c2, c4, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c5, c2, c4, x0), 0.0);
  for (auto& x1 : *range_[1]) {
    // tensor label: Gamma27
    std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x1, x0);
    std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x1, x0)]);
    sort_indices<0,1,0,1,1,1>(i0data, i0data_sorted, x1.size(), x0.size());
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x1, c5, c2, c4);
    std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x1, c5, c2, c4)]);
    sort_indices<0,1,2,3,0,1,-2,1>(i1data, i1data_sorted, x1.size(), c5.size(), c2.size(), c4.size());
    zgemm3m_("T", "N", x0.size(), c5.size()*c2.size()*c4.size(), x1.size(),
           1.0, i0data_sorted, x1.size(), i1data_sorted, x1.size(),
           1.0, odata_sorted, x0.size());
  }
  sort_indices<1,2,3,0,1,1,1,1>(odata_sorted, odata, x0.size(), c5.size(), c2.size(), c4.size());
  out()->put_block(odata, c5, c2, c4, x0);
}

void Task631::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& c4 : *range_[0]) {
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, a3, c4, a1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, a3, c4, a1)]);
      sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a3.size(), c4.size(), a1.size());
      // tensor label: I1101
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, c4, x3, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, c4, x3, x0)]);
      sort_indices<2,1,0,3,0,1,1,1>(i1data, i1data_sorted, c2.size(), c4.size(), x3.size(), x0.size());
      zgemm3m_("T", "N", a3.size()*a1.size(), c2.size()*x0.size(), c4.size()*x3.size(),
             1.0, i0data_sorted, c4.size()*x3.size(), i1data_sorted, c4.size()*x3.size(),
             1.0, odata_sorted, a3.size()*a1.size());
    }
  }
  sort_indices<2,0,3,1,1,1,1,1>(odata_sorted, odata, a3.size(), a1.size(), c2.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task632::Task_local::compute() {
  const Index c2 = b(0);
  const Index c4 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1101
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, c4, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, c4, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, c4, x3, x0), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: Gamma33
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i0data, i0data_sorted, x3.size(), x0.size(), x2.size(), x1.size());
      // tensor label: I1102
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, c4, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, c4, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, c2.size(), c4.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), c2.size()*c4.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), c2.size(), c4.size());
  out()->put_block(odata, c2, c4, x3, x0);
}

void Task633::Task_local::compute() {
  const Index c2 = b(0);
  const Index c4 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1102
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, c4, x2, x1);
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c2, c4, x2, x1);
    sort_indices<0,1,2,3,1,1,1,2>(i0data, odata, c2.size(), c4.size(), x2.size(), x1.size());
  }
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i1data = in(0)->get_block(x2, x1, c2, c4);
    sort_indices<2,3,0,1,1,1,1,2>(i1data, odata, x2.size(), x1.size(), c2.size(), c4.size());
  }
  out()->put_block(odata, c2, c4, x2, x1);
}

void Task634::Task_local::compute() {
  const Index c2 = b(0);
  const Index c4 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1101
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, c4, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, c4, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, c4, x3, x0), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: Gamma24
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x2, x1, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x2, x1, x0)]);
      sort_indices<1,2,0,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x2.size(), x1.size(), x0.size());
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, x2, x1, c4);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, x2, x1, c4)]);
      sort_indices<1,2,0,3,0,1,1,2>(i1data, i1data_sorted, c2.size(), x2.size(), x1.size(), c4.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), c2.size()*c4.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), c2.size(), c4.size());
  out()->put_block(odata, c2, c4, x3, x0);
}

void Task635::Task_local::compute() {
  const Index c2 = b(0);
  const Index c4 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1101
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, c4, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, c4, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, c4, x3, x0), 0.0);
  for (auto& x1 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      // tensor label: Gamma368
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x1, x2, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x1, x2, x0)]);
      sort_indices<1,2,0,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x1.size(), x2.size(), x0.size());
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x2, c4, c2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x2, c4, c2, x1)]);
      sort_indices<3,0,1,2,0,1,-1,2>(i1data, i1data_sorted, x2.size(), c4.size(), c2.size(), x1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), c4.size()*c2.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<3,2,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), c4.size(), c2.size());
  out()->put_block(odata, c2, c4, x3, x0);
}

void Task636::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& c4 : *range_[0]) {
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, a1, c4, a3);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, a1, c4, a3)]);
      sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), c4.size(), a3.size());
      // tensor label: I1104
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, c4, x3, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, c4, x3, x0)]);
      sort_indices<2,1,0,3,0,1,1,1>(i1data, i1data_sorted, c2.size(), c4.size(), x3.size(), x0.size());
      zgemm3m_("T", "N", a1.size()*a3.size(), c2.size()*x0.size(), c4.size()*x3.size(),
             1.0, i0data_sorted, c4.size()*x3.size(), i1data_sorted, c4.size()*x3.size(),
             1.0, odata_sorted, a1.size()*a3.size());
    }
  }
  sort_indices<2,1,3,0,1,1,1,1>(odata_sorted, odata, a1.size(), a3.size(), c2.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task637::Task_local::compute() {
  const Index c2 = b(0);
  const Index c4 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1104
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, c4, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, c4, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, c4, x3, x0), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: Gamma33
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i0data, i0data_sorted, x3.size(), x0.size(), x2.size(), x1.size());
      // tensor label: I1105
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, c4, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, c4, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, c2.size(), c4.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), c2.size()*c4.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), c2.size(), c4.size());
  out()->put_block(odata, c2, c4, x3, x0);
}

void Task638::Task_local::compute() {
  const Index c2 = b(0);
  const Index c4 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1105
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, c4, x2, x1);
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(c2, c4, x2, x1);
    sort_indices<0,1,2,3,1,1,-1,2>(i0data, odata, c2.size(), c4.size(), x2.size(), x1.size());
  }
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i1data = in(0)->get_block(x2, c4, c2, x1);
    sort_indices<2,1,0,3,1,1,1,2>(i1data, odata, x2.size(), c4.size(), c2.size(), x1.size());
  }
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i2data = in(0)->get_block(x2, x1, c2, c4);
    sort_indices<2,3,0,1,1,1,-1,2>(i2data, odata, x2.size(), x1.size(), c2.size(), c4.size());
  }
  out()->put_block(odata, c2, c4, x2, x1);
}

void Task639::Task_local::compute() {
  const Index c2 = b(0);
  const Index c4 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1104
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, c4, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, c4, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, c4, x3, x0), 0.0);
  for (auto& x1 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      // tensor label: Gamma363
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x0, x1, x2);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x0, x1, x2)]);
      sort_indices<2,3,0,1,0,1,1,1>(i0data, i0data_sorted, x3.size(), x0.size(), x1.size(), x2.size());
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(c2, x2, x1, c4);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(c2, x2, x1, c4)]);
      sort_indices<2,1,0,3,0,1,-1,2>(i1data, i1data_sorted, c2.size(), x2.size(), x1.size(), c4.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), c2.size()*c4.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), c2.size(), c4.size());
  out()->put_block(odata, c2, c4, x3, x0);
}

void Task640::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& a4 : *range_[2]) {
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, a4, c2, a3);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, a4, c2, a3)]);
      sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a4.size(), c2.size(), a3.size());
      // tensor label: I1107
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a4, a1, x3, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a4, a1, x3, x0)]);
      sort_indices<2,0,1,3,0,1,1,1>(i1data, i1data_sorted, a4.size(), a1.size(), x3.size(), x0.size());
      zgemm3m_("T", "N", c2.size()*a3.size(), a1.size()*x0.size(), a4.size()*x3.size(),
             1.0, i0data_sorted, a4.size()*x3.size(), i1data_sorted, a4.size()*x3.size(),
             1.0, odata_sorted, c2.size()*a3.size());
    }
  }
  sort_indices<0,1,3,2,1,1,1,1>(odata_sorted, odata, c2.size(), a3.size(), a1.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task641::Task_local::compute() {
  const Index a4 = b(0);
  const Index a1 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1107
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, a1, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a4, a1, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a4, a1, x3, x0), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: Gamma33
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i0data, i0data_sorted, x3.size(), x0.size(), x2.size(), x1.size());
      // tensor label: I1108
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a4, a1, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a4, a1, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, a4.size(), a1.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), a4.size()*a1.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), a4.size(), a1.size());
  out()->put_block(odata, a4, a1, x3, x0);
}

void Task642::Task_local::compute() {
  const Index a4 = b(0);
  const Index a1 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1108
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, a1, x2, x1);
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(a4, a1, x2, x1);
    sort_indices<0,1,2,3,1,1,1,2>(i0data, odata, a4.size(), a1.size(), x2.size(), x1.size());
  }
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i1data = in(0)->get_block(x2, x1, a4, a1);
    sort_indices<2,3,0,1,1,1,1,2>(i1data, odata, x2.size(), x1.size(), a4.size(), a1.size());
  }
  out()->put_block(odata, a4, a1, x2, x1);
}

void Task643::Task_local::compute() {
  const Index a4 = b(0);
  const Index a1 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1107
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, a1, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a4, a1, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a4, a1, x3, x0), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: Gamma24
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x2, x1, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x2, x1, x0)]);
      sort_indices<1,2,0,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x2.size(), x1.size(), x0.size());
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a4, x2, x1, a1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a4, x2, x1, a1)]);
      sort_indices<1,2,0,3,0,1,1,2>(i1data, i1data_sorted, a4.size(), x2.size(), x1.size(), a1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), a4.size()*a1.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), a4.size(), a1.size());
  out()->put_block(odata, a4, a1, x3, x0);
}

void Task644::Task_local::compute() {
  const Index a4 = b(0);
  const Index a1 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1107
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, a1, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a4, a1, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a4, a1, x3, x0), 0.0);
  for (auto& x1 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      // tensor label: Gamma368
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x1, x2, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x1, x2, x0)]);
      sort_indices<1,2,0,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x1.size(), x2.size(), x0.size());
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x2, a1, a4, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x2, a1, a4, x1)]);
      sort_indices<3,0,1,2,0,1,-1,2>(i1data, i1data_sorted, x2.size(), a1.size(), a4.size(), x1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), a1.size()*a4.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<3,2,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), a1.size(), a4.size());
  out()->put_block(odata, a4, a1, x3, x0);
}

void Task645::Task_local::compute() {
  const Index c2 = b(0);
  const Index a3 = b(1);
  const Index x0 = b(2);
  const Index a1 = b(3);
  // tensor label: I134
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(c2, a3, x0, a1);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(c2, a3, x0, a1)]);
  std::fill_n(odata_sorted.get(), out()->get_size(c2, a3, x0, a1), 0.0);
  for (auto& x3 : *range_[1]) {
    for (auto& a4 : *range_[2]) {
      // tensor label: t2
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, a3, c2, a4);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, a3, c2, a4)]);
      sort_indices<0,3,1,2,0,1,1,1>(i0data, i0data_sorted, x3.size(), a3.size(), c2.size(), a4.size());
      // tensor label: I1110
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a4, a1, x3, x0);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a4, a1, x3, x0)]);
      sort_indices<2,0,1,3,0,1,1,1>(i1data, i1data_sorted, a4.size(), a1.size(), x3.size(), x0.size());
      zgemm3m_("T", "N", a3.size()*c2.size(), a1.size()*x0.size(), a4.size()*x3.size(),
             1.0, i0data_sorted, a4.size()*x3.size(), i1data_sorted, a4.size()*x3.size(),
             1.0, odata_sorted, a3.size()*c2.size());
    }
  }
  sort_indices<1,0,3,2,1,1,1,1>(odata_sorted, odata, a3.size(), c2.size(), a1.size(), x0.size());
  out()->put_block(odata, c2, a3, x0, a1);
}

void Task646::Task_local::compute() {
  const Index a4 = b(0);
  const Index a1 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1110
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, a1, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a4, a1, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a4, a1, x3, x0), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: Gamma33
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x0, x2, x1);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x0, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i0data, i0data_sorted, x3.size(), x0.size(), x2.size(), x1.size());
      // tensor label: I1111
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a4, a1, x2, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a4, a1, x2, x1)]);
      sort_indices<2,3,0,1,0,1,1,1>(i1data, i1data_sorted, a4.size(), a1.size(), x2.size(), x1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), a4.size()*a1.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), a4.size(), a1.size());
  out()->put_block(odata, a4, a1, x3, x0);
}

void Task647::Task_local::compute() {
  const Index a4 = b(0);
  const Index a1 = b(1);
  const Index x2 = b(2);
  const Index x1 = b(3);
  // tensor label: I1111
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, a1, x2, x1);
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(a4, a1, x2, x1);
    sort_indices<0,1,2,3,1,1,-1,2>(i0data, odata, a4.size(), a1.size(), x2.size(), x1.size());
  }
  {
    // tensor label: v2
    std::unique_ptr<std::complex<double>[]> i1data = in(0)->get_block(x2, x1, a4, a1);
    sort_indices<2,3,0,1,1,1,-1,2>(i1data, odata, x2.size(), x1.size(), a4.size(), a1.size());
  }
  out()->put_block(odata, a4, a1, x2, x1);
}

void Task648::Task_local::compute() {
  const Index a4 = b(0);
  const Index a1 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1110
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, a1, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a4, a1, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a4, a1, x3, x0), 0.0);
  for (auto& x2 : *range_[1]) {
    for (auto& x1 : *range_[1]) {
      // tensor label: Gamma24
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x2, x1, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x2, x1, x0)]);
      sort_indices<1,2,0,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x2.size(), x1.size(), x0.size());
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(a4, x2, x1, a1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(a4, x2, x1, a1)]);
      sort_indices<1,2,0,3,0,1,-1,2>(i1data, i1data_sorted, a4.size(), x2.size(), x1.size(), a1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), a4.size()*a1.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), a4.size(), a1.size());
  out()->put_block(odata, a4, a1, x3, x0);
}

void Task649::Task_local::compute() {
  const Index a4 = b(0);
  const Index a1 = b(1);
  const Index x3 = b(2);
  const Index x0 = b(3);
  // tensor label: I1110
  std::unique_ptr<std::complex<double>[]> odata = out()->move_block(a4, a1, x3, x0);
  std::unique_ptr<std::complex<double>[]> odata_sorted(new std::complex<double>[out()->get_size(a4, a1, x3, x0)]);
  std::fill_n(odata_sorted.get(), out()->get_size(a4, a1, x3, x0), 0.0);
  for (auto& x1 : *range_[1]) {
    for (auto& x2 : *range_[1]) {
      // tensor label: Gamma368
      std::unique_ptr<std::complex<double>[]> i0data = in(0)->get_block(x3, x1, x2, x0);
      std::unique_ptr<std::complex<double>[]> i0data_sorted(new std::complex<double>[in(0)->get_size(x3, x1, x2, x0)]);
      sort_indices<1,2,0,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), x1.size(), x2.size(), x0.size());
      // tensor label: v2
      std::unique_ptr<std::complex<double>[]> i1data = in(1)->get_block(x2, a1, a4, x1);
      std::unique_ptr<std::complex<double>[]> i1data_sorted(new std::complex<double>[in(1)->get_size(x2, a1, a4, x1)]);
      sort_indices<3,0,1,2,0,1,1,2>(i1data, i1data_sorted, x2.size(), a1.size(), a4.size(), x1.size());
      zgemm3m_("T", "N", x3.size()*x0.size(), a1.size()*a4.size(), x2.size()*x1.size(),
             1.0, i0data_sorted, x2.size()*x1.size(), i1data_sorted, x2.size()*x1.size(),
             1.0, odata_sorted, x3.size()*x0.size());
    }
  }
  sort_indices<3,2,0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x0.size(), a1.size(), a4.size());
  out()->put_block(odata, a4, a1, x3, x0);
}

#endif