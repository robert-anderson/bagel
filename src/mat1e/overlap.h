//
// BAGEL - Brilliantly Advanced General Electronic Structure Library
// Filename: overlap.h
// Copyright (C) 2009 Quantum Simulation Technologies, Inc.
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


#ifndef __SRC_MAT1E_OVERLAP_H
#define __SRC_MAT1E_OVERLAP_H

#include <src/mat1e/matrix1e.h>

namespace bagel {

class Overlap : public Matrix1e {
  protected:
    void computebatch(const std::array<std::shared_ptr<const Shell>,2>&, const int, const int, std::shared_ptr<const Molecule>) override;

  private:
    // serialization
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
      ar & boost::serialization::base_object<Matrix1e>(*this);
    }

  public:
    Overlap() { }
    Overlap(std::shared_ptr<const Molecule>);

};

}

#include <src/util/archive.h>
BOOST_CLASS_EXPORT_KEY(bagel::Overlap)

#endif

