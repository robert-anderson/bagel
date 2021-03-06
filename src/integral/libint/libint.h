//
// BAGEL - Brilliantly Advanced General Electronic Structure Library
// Filename: libint.h
// Copyright (C) 2012 Quantum Simulation Technologies, Inc.
//
// Author: Toru Shiozaki <shiozaki@qsimulate.com>
// Maintainer: QSimulate
//
// This file is part of the BAGEL package.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


#ifdef LIBINT_INTERFACE

#ifndef __SRC_RYSINT_LIBINT_H
#define __SRC_RYSINT_LIBINT_H

#include <src/integral/rys/rysintegral.h>

namespace bagel {

class Libint : public RysIntegral<double> {
  protected:
    void root_weight(int) {}
    void compute_ssss(double) {}

  public:
    Libint(const std::array<std::shared_ptr<const Shell>,4>&, const double dum = 0.0, std::shared_ptr<StackMem> stack = nullptr);

    void compute() {}

    constexpr static int Nblocks() { return 1; }

    virtual void allocate_data(const int asize_final, const int csize_final, const int asize_final_sph, const int csize_final_sph) { assert(false); }
};

}

#endif

#endif
