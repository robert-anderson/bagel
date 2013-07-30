//
// BAGEL - Parallel electron correlation program.
// Filename: gsmalleribatch.cc
// Copyright (C) 2013 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and/or modify
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

#include <iostream>
#include <iomanip>
#include <src/rel/alpha.h>
#include <src/integral/rys/gradbatch.h>
#include <src/integral/libint/glibint.h>
#include <src/integral/rys/gsmalleribatch.h>

using namespace std;
using namespace bagel;

GSmallERIBatch::GSmallERIBatch(std::array<std::shared_ptr<const Shell>,4> info, array<int,3> atoms, const int natoms)
 : shells_{{info[1],info[2],info[3]}}, atoms_(atoms), natoms_(natoms), stack_(resources__->get()) {

  // shells_[0] is aux function, shelles_[1] and [2] are basis
  s0size_ = shells_[0]->nbasis();
  a1size_inc_ = shells_[1]->aux_inc()->nbasis();
  a2size_inc_ = shells_[2]->aux_inc()->nbasis();
  a1size_dec_ = shells_[1]->aux_dec() ? shells_[1]->aux_dec()->nbasis() : 0;
  a2size_dec_ = shells_[2]->aux_dec() ? shells_[2]->aux_dec()->nbasis() : 0;
  a1_ = a1size_inc_ + a1size_dec_;
  a2_ = a2size_inc_ + a2size_dec_;

  assert(info[0]->dummy());

  // we store primitive integrals
  size_block_ = s0size_*a1_*a2_; 
  size_alloc_ = size_block_*9;
  data_ = stack_->get(size_alloc_);
}


GSmallERIBatch::~GSmallERIBatch() {
  stack_->release(size_alloc_, data_);
  resources__->release(stack_);
}


std::shared_ptr<GradFile> GSmallERIBatch::compute_gradient(array<unique_ptr<double[]>,6>& d) const {
  auto out = make_shared<GradFile>(natoms_);
  static_assert(Comp::X == 0 && Comp::Y == 1 && Comp::Z == 2, "something is wrong in GSmallERIBatch::compute_gradient");

  const size_t s1size = shells_[1]->nbasis();
  const size_t s2size = shells_[2]->nbasis();

  double* denc = stack_->get(s0size_*a1_*a2_);
  fill_n(denc, s0size_*a1_*a2_, 0.0);

  double* tmp = stack_->get(s0size_*s1size*a2_);

  int cnt = 0;
  array<int,3> xyz{{Comp::X, Comp::Y, Comp::Z}};
  for (int& i : xyz) {
    for (int& j : xyz) {
      if (i <= j) {
        dgemm_("N", "T", s0size_*s1size, a2_, s2size, 1.0, d[cnt++].get(), s0size_*s1size, shells_[2]->small(j)->data(), a2_, 0.0, tmp, s0size_*s1size);
        for (int a = 0; a != a2_; ++a) {
          dgemm_("N", "T", s0size_, a1_, s1size, 1.0, tmp+a*s0size_*s1size, s0size_, shells_[1]->small(i)->data(), a1_, 1.0, denc+a*s0size_*a1_, s0size_); 
        }
      }
    }
  }

  for (int iatom = 0; iatom != 3; ++iatom) {
    for (int i = 0; i != 3; ++i) {
      out->data(atoms_[iatom]*3+i) += ddot_(s0size_*a1_*a2_, denc, 1, data(iatom*3+i), 1);
    }
  }

  stack_->release(s0size_*s1size*a2_, tmp);
  stack_->release(s0size_*a1_*a2_, denc);
  return out;
}


void GSmallERIBatch::compute() {


  auto dummy = make_shared<const Shell>(shells_[0]->spherical());

  struct Address {
    const size_t ld0, ld1, ld2;
    Address(const size_t l0, const size_t l1, const size_t l2) : ld0(l0), ld1(l1), ld2(l2) {}
    size_t operator()(const size_t& i, const size_t& j, const size_t& k) const { return i+ld0*(j+ld1*k); }
  } m(s0size_, a1_, a2_);

  {
#ifndef LIBINT_INTERFACE
    auto eric = make_shared<GradBatch>(array<shared_ptr<const Shell>,4>{{dummy, shells_[0], shells_[1]->aux_inc(), shells_[2]->aux_inc()}}, 2.0, 0.0, true, stack_);
#else
    auto eric = make_shared<GLibint>(array<shared_ptr<const Shell>,4>{{dummy, shells_[0], shells_[1]->aux_inc(), shells_[2]->aux_inc()}}, stack_);
#endif
    eric->compute();

    array<int,4> jatom = {{-1, 2, 1, 0}};
    if (eric->swap0123()) { swap(jatom[0], jatom[2]); swap(jatom[1], jatom[3]); }
    if (eric->swap01()) swap(jatom[0], jatom[1]);
    if (eric->swap23()) swap(jatom[2], jatom[3]);
    for (int iatom = 0; iatom != 4; ++iatom) {
      if (jatom[iatom] < 0) continue;
      for (int icart = 0; icart != 3; ++icart) {
        const int sblock = icart + iatom*3;
        const int iblock = icart + jatom[iatom]*3;
        for (int i = 0; i != a2size_inc_; i++)
          copy_n(eric->data(sblock) + i*s0size_*a1size_inc_, s0size_*a1size_inc_, data(iblock) + m(0,0,i));
      }
    }
  }
  if (shells_[1]->aux_dec() && shells_[2]->aux_dec()) {
#ifndef LIBINT_INTERFACE
    auto eric = make_shared<GradBatch>(array<shared_ptr<const Shell>,4>{{dummy, shells_[0], shells_[1]->aux_dec(), shells_[2]->aux_dec()}}, 2.0, 0.0, true, stack_);
#else
    auto eric = make_shared<GLibint>(array<shared_ptr<const Shell>,4>{{dummy, shells_[0], shells_[1]->aux_dec(), shells_[2]->aux_dec()}}, stack_);
#endif
    eric->compute();

    array<int,4> jatom = {{-1, 2, 1, 0}};
    if (eric->swap0123()) { swap(jatom[0], jatom[2]); swap(jatom[1], jatom[3]); }
    if (eric->swap01()) swap(jatom[0], jatom[1]);
    if (eric->swap23()) swap(jatom[2], jatom[3]);
    for (int iatom = 0; iatom != 4; ++iatom) {
      if (jatom[iatom] < 0) continue;
      for (int icart = 0; icart != 3; ++icart) {
        const int sblock = icart + iatom*3;
        const int iblock = icart + jatom[iatom]*3;
        for (int i = 0; i != a2size_dec_; i++)
          copy_n(eric->data(sblock) + i*s0size_*a1size_dec_, s0size_*a1size_dec_, data(iblock) + m(0,a1size_inc_,a2size_inc_+i));
      }
    }
  }
  if (shells_[1]->aux_dec()) {
#ifndef LIBINT_INTERFACE
    auto eric = make_shared<GradBatch>(array<shared_ptr<const Shell>,4>{{dummy, shells_[0], shells_[1]->aux_dec(), shells_[2]->aux_inc()}}, 2.0, 0.0, true, stack_);
#else
    auto eric = make_shared<GLibint>(array<shared_ptr<const Shell>,4>{{dummy, shells_[0], shells_[1]->aux_dec(), shells_[2]->aux_inc()}}, stack_);
#endif
    eric->compute();

    array<int,4> jatom = {{-1, 2, 1, 0}};
    if (eric->swap0123()) { swap(jatom[0], jatom[2]); swap(jatom[1], jatom[3]); }
    if (eric->swap01()) swap(jatom[0], jatom[1]);
    if (eric->swap23()) swap(jatom[2], jatom[3]);
    for (int iatom = 0; iatom != 4; ++iatom) {
      if (jatom[iatom] < 0) continue;
      for (int icart = 0; icart != 3; ++icart) {
        const int sblock = icart + iatom*3;
        const int iblock = icart + jatom[iatom]*3;
        for (int i = 0; i != a2size_inc_; i++)
          copy_n(eric->data(sblock) + i*s0size_*a1size_dec_, s0size_*a1size_dec_, data(iblock) + m(0,a1size_inc_, i));
      }
    }
  }
  if (shells_[2]->aux_dec()) {
#ifndef LIBINT_INTERFACE
    auto eric = make_shared<GradBatch>(array<shared_ptr<const Shell>,4>{{dummy, shells_[0], shells_[1]->aux_inc(), shells_[2]->aux_dec()}}, 2.0, 0.0, true, stack_);
#else
    auto eric = make_shared<GLibint>(array<shared_ptr<const Shell>,4>{{dummy, shells_[0], shells_[1]->aux_inc(), shells_[2]->aux_dec()}}, stack_);
#endif
    eric->compute();

    array<int,4> jatom = {{-1, 2, 1, 0}};
    if (eric->swap0123()) { swap(jatom[0], jatom[2]); swap(jatom[1], jatom[3]); }
    if (eric->swap01()) swap(jatom[0], jatom[1]);
    if (eric->swap23()) swap(jatom[2], jatom[3]);
    for (int iatom = 0; iatom != 4; ++iatom) {
      if (jatom[iatom] < 0) continue;
      for (int icart = 0; icart != 3; ++icart) {
        const int sblock = icart + iatom*3;
        const int iblock = icart + jatom[iatom]*3;
        for (int i = 0; i != a2size_dec_; i++)
          copy_n(eric->data(sblock) + i*s0size_*a1size_inc_, s0size_*a1size_inc_, data(iblock) + m(0,0,a2size_inc_+i));
      }
    }
  }
}
