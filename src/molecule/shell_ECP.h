//
// BAGEL - Parallel electron correlation program.
// Filename: shell_ECP.h
// Copyright (C) 2009 Toru Shiozaki
//
// Author: Hai-Anh Le <anh@u.northwestern.edu>
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


#ifndef __SRC_MOLECULE_SHELL_ECP_H
#define __SRC_MOLECULE_SHELL_ECP_H

#include <src/molecule/shell_base.h>

namespace bagel {

class Shell_ECP : public Shell_base {

  protected:
    int ecp_ncore_;
    std::vector<double> ecp_exponents_;
    std::vector<double> ecp_coefficients_;
    std::vector<int> ecp_r_power_;

  public:
    Shell_ECP() { }

    Shell_ECP(const std::array<double,3>& position, const int ncore, const int angular_num, const std::vector<double>& ecp_exponents,
              const std::vector<double>& ecp_coefficients, const std::vector<int>& ecp_r_power);

    int ecp_ncore() const { return ecp_ncore_; }

    double ecp_exponents(const int i) const { return ecp_exponents_[i]; }
    const std::vector<double>& ecp_exponents() const { return ecp_exponents_; }
    const double* ecp_exponents_pointer() const { return &(ecp_exponents_[0]); }

    double ecp_coefficients(const int i) const { return ecp_coefficients_[i]; }
    const std::vector<double>& ecp_coefficients() const { return ecp_coefficients_; }
    const double* ecp_coefficients_pointer() const { return &(ecp_coefficients_[0]); }

    std::string show() const override;

};

}

#endif

