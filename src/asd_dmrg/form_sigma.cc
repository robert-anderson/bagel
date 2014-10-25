//
// BAGEL - Parallel electron correlation program.
// Filename: asd_dmrg/form_sigma.cc
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Shane Parker <shane.parker@u.northwestern.edu>
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

#include <map>

#include <src/math/sparsematrix.h>
#include <src/math/blocksparsematrix.h>
#include <src/asd_dmrg/form_sigma.h>
#include <src/ras/form_sigma.h>
#include <src/ras/apply_operator.h>

using namespace std;
using namespace bagel;

vector<shared_ptr<ProductRASCivec>> FormSigmaProdRAS::operator()(const vector<shared_ptr<ProductRASCivec>>& ccvec,
                     shared_ptr<const BlockOperators> blockops, shared_ptr<DimerJop> jop, const vector<bool>& conv) const {

  const int nstate = ccvec.size();
  vector<shared_ptr<ProductRASCivec>> sigmavec;
  for_each(ccvec.begin(), ccvec.end(), [&sigmavec] (shared_ptr<const ProductRASCivec> c) { sigmavec.push_back(c->clone()); });

  for (int istate = 0; istate != nstate; ++istate) {
    Timer pdebug(2);
    shared_ptr<const ProductRASCivec> cc = ccvec.at(istate);
    shared_ptr<ProductRASCivec> sigma = sigmavec.at(istate);

    // pure terms
    pure_block_and_ras(cc, sigma, blockops, jop);
    pdebug.tick_print("pure");

    interaction_terms(cc, sigma, blockops, jop);
    pdebug.tick_print("interaction");
  }

#ifdef HAVE_MPI_H
  for (int istate = 0; istate != nstate; ++istate) {
    if (!conv[istate])
      sigmavec.at(istate)->allreduce();
  }
#endif

  return sigmavec;
}

vector<shared_ptr<ProductRASCivec>> FormSigmaProdRAS::diagonal(const vector<shared_ptr<ProductRASCivec>>& ccvec,
                     shared_ptr<const BlockOperators> blockops, shared_ptr<DimerJop> jop, const vector<bool>& conv) const {

  const int nstate = ccvec.size();
  vector<shared_ptr<ProductRASCivec>> sigmavec;
  for_each(ccvec.begin(), ccvec.end(), [&sigmavec] (shared_ptr<const ProductRASCivec> c) { sigmavec.push_back(c->clone()); });

  for (int istate = 0; istate != nstate; ++istate) {
    Timer pdebug(2);
    shared_ptr<const ProductRASCivec> cc = ccvec.at(istate);
    shared_ptr<ProductRASCivec> sigma = sigmavec.at(istate);

    // pure terms
    pure_block_and_ras(cc, sigma, blockops, jop);
    pdebug.tick_print("pure");

    diagonal_terms(cc, sigma, blockops, jop);
    pdebug.tick_print("diagonal");
  }

#ifdef HAVE_MPI_H
  for (int istate = 0; istate != nstate; ++istate) {
    if (!conv[istate])
      sigmavec.at(istate)->allreduce();
  }
#endif

  return sigmavec;
}

void FormSigmaProdRAS::pure_block_and_ras(shared_ptr<const ProductRASCivec> cc, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops, shared_ptr<DimerJop> jop) const {
  Timer ptime(2);
  for (auto& sector : sigma->sectors()) {
    // first prepare pure block part which will be a nsecstates x nsecstates matrix
    const int nsecstates = sector.second->nstates();
    const Matrix pure_block = *blockops->ham(sector.first);
    assert(pure_block.ndim()==nsecstates && pure_block.mdim()==nsecstates);

    shared_ptr<RASBlockVectors> sigma_sector = sector.second;
    shared_ptr<const RASBlockVectors> cc_sector = cc->sector(sector.first);
    // TODO: would this benefit from being blocksparse?
    if (mpi__->rank() == 0) {
      dgemm_("N","T", sigma_sector->ndim(), sigma_sector->mdim(), sigma_sector->mdim(), 1.0, cc_sector->data(), cc_sector->ndim(), pure_block.data(), pure_block.ndim(),
                                                                                        0.0, sigma_sector->data(), sigma_sector->ndim());
    }
    ptime.tick_print("pure_block");

    // now do individual form_sigmas for the RAS parts
    FormSigmaRAS form_pure_ras(batchsize_);
    for(int ist = 0; ist < nsecstates; ++ist) {
#ifdef HAVE_MPI_H
      if (ist%mpi__->size() == mpi__->rank())
        form_pure_ras(cc_sector->civec(ist), sigma_sector->civec(ist), jop->monomer_jop<0>());
#else
      form_pure_ras(cc_sector->civec(ist), sigma_sector->civec(ist), jop->monomer_jop<0>());
#endif
    }
    ptime.tick_print("pure_ras");
  }
}


void FormSigmaProdRAS::diagonal_terms(shared_ptr<const ProductRASCivec> cc, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops, shared_ptr<DimerJop> jop) const {
  Timer ptime(2);

  for (auto& isec : cc->sectors()) {
    shared_ptr<const RASBlockVectors> cc_sector = isec.second;

    aexc_branch(cc_sector, sigma, blockops);
    bexc_branch(cc_sector, sigma, blockops);
  }
}


void FormSigmaProdRAS::interaction_terms(shared_ptr<const ProductRASCivec> cc, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops, shared_ptr<DimerJop> jop) const {
  Timer ptime(2);

  for (auto& isec : cc->sectors()) {
    shared_ptr<const RASBlockVectors> cc_sector = isec.second;
    const BlockKey cc_key = isec.first;

    // really ugly but verbose
    const bool do_aET    = cc->contains_block(BlockKey(cc_key.nelea-1, cc_key.neleb  ));
    const bool do_bET    = cc->contains_block(BlockKey(cc_key.nelea  , cc_key.neleb-1));
    const bool do_aHT    = cc->contains_block(BlockKey(cc_key.nelea+1, cc_key.neleb  ));
    const bool do_bHT    = cc->contains_block(BlockKey(cc_key.nelea  , cc_key.neleb+1));
    const bool do_aaET   = cc->contains_block(BlockKey(cc_key.nelea-2, cc_key.neleb  ));
    const bool do_bbET   = cc->contains_block(BlockKey(cc_key.nelea  , cc_key.neleb-2));
    const bool do_abET   = cc->contains_block(BlockKey(cc_key.nelea-1, cc_key.neleb-1));
    const bool do_aaHT   = cc->contains_block(BlockKey(cc_key.nelea+2, cc_key.neleb  ));
    const bool do_bbHT   = cc->contains_block(BlockKey(cc_key.nelea  , cc_key.neleb+2));
    const bool do_abHT   = cc->contains_block(BlockKey(cc_key.nelea+1, cc_key.neleb+1));
    const bool do_flipup = cc->contains_block(BlockKey(cc_key.nelea-1, cc_key.neleb+1));
    const bool do_flipdn = cc->contains_block(BlockKey(cc_key.nelea+1, cc_key.neleb-1));

    if (do_aET || do_aaET) {
      aET_branch(cc_sector, sigma, blockops);
      ptime.tick_print("aET-branch");
    }

    if (do_bET || do_bbET || do_abET) {
      bET_branch(cc_sector, sigma, blockops);
      ptime.tick_print("bET-branch");
    }

    if (do_aHT || do_aaHT) {
      aHT_branch(cc_sector, sigma, blockops);
      ptime.tick_print("aHT-branch");
    }

    if (do_bHT || do_bbHT || do_abHT) {
      bHT_branch(cc_sector, sigma, blockops);
      ptime.tick_print("bHT-branch");
    }

    // always compute these
    aexc_branch(cc_sector, sigma, blockops);
    bexc_branch(cc_sector, sigma, blockops);
    ptime.tick_print("exc-branches");

    if (do_flipup) {
      abflip_branch(cc_sector, sigma, blockops);
      ptime.tick_print("abflip-branch");
    }

    if (do_flipdn) {
      baflip_branch(cc_sector, sigma, blockops);
      ptime.tick_print("baflip-branch");
    }

    if (do_aET) {
      compute_sigma_3aET(cc_sector, sigma, blockops, jop);
      ptime.tick_print("sigma-3aET");
    }

    if (do_aHT) {
      compute_sigma_3aHT(cc_sector, sigma, blockops, jop);
      ptime.tick_print("sigma-3aHT");
    }

    if (do_bET) {
      compute_sigma_3bET(cc_sector, sigma, blockops, jop);
      ptime.tick_print("sigma-3bET");
    }

    if (do_bHT) {
      compute_sigma_3bHT(cc_sector, sigma, blockops, jop);
      ptime.tick_print("sigma-3bHT");
    }
  }
}

void FormSigmaProdRAS::aET_branch(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops) const {
  ApplyOperator apply;
  const int rnorb = cc_sector->det()->norb();
  // S_alpha^+
  const BlockKey cckey = cc_sector->left_state().key();

  const BlockKey singleETkey(cckey.nelea-1, cckey.neleb);
  const BlockKey doubleETkey(cckey.nelea-2, cckey.neleb);

  const bool do_single = sigma->contains_block(singleETkey);
  const bool do_double = sigma->contains_block(doubleETkey);
  // not sure how you could do a double but not a single, but that's a different problem
  assert(do_single || do_double);

  const int nccstates = cc_sector->nstates();

  shared_ptr<RASBlockVectors> single_sector = do_single ? sigma->sector(singleETkey) : nullptr;
  shared_ptr<const RASDeterminants> single_det = do_single ? single_sector->det() : sigma->space()->det(singleETkey.nelea, singleETkey.neleb);

  shared_ptr<RASBlockVectors> double_sector = do_double ? sigma->sector(doubleETkey) : nullptr;
  shared_ptr<RASBlockVectors> tmp_double = do_double ? make_shared<RASBlockVectors>(double_sector->det(), BlockInfo(doubleETkey.nelea, doubleETkey.neleb, nccstates))
                                                     : nullptr;

  const BlockInfo singlestate(singleETkey.nelea, singleETkey.neleb, nccstates);
  RASBlockVectors sector_r(single_det, BlockInfo(singleETkey.nelea, singleETkey.neleb, nccstates));

  const int phase = (1 - (((cc_sector->det()->nelea()+cc_sector->det()->neleb())%2) << 1));

#ifdef HAVE_MPI_H
  int mpi_counter = 0;
#endif

  for (int r = 0; r < rnorb; ++r) {
    sector_r.zero();
    apply(1.0, *cc_sector, sector_r, {GammaSQ::CreateAlpha}, {r});
    if (do_single) {
#ifdef HAVE_MPI_H
      if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
        shared_ptr<const BlockSparseMatrix> Sr = blockops->S_a(singleETkey, r);
        mat_block_multiply(false, false, phase, sector_r, *Sr, 1.0, *single_sector);
#ifdef HAVE_MPI_H
      }
#endif
    }
    if (do_double) {
      for (int s = 0; s < r; ++s) {
#ifdef HAVE_MPI_H
        if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
          tmp_double->zero();
          apply(1.0, sector_r, *tmp_double, {GammaSQ::CreateAlpha}, {s});

          shared_ptr<const BlockSparseMatrix> Prs = blockops->P_aa(cckey, s, r);
          mat_block_multiply(false, true, 2.0, *tmp_double, *Prs, 1.0, *double_sector);
#ifdef HAVE_MPI_H
        }
#endif
      }
    }
  }
}

void FormSigmaProdRAS::bET_branch(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops) const {
  ApplyOperator apply;
  const int rnorb = cc_sector->det()->norb();

  // S_beta^+
  const BlockKey cckey = cc_sector->left_state().key();

  const BlockKey bETkey(cckey.nelea, cckey.neleb-1);
  const BlockKey bbETkey(cckey.nelea, cckey.neleb-2);
  const BlockKey abETkey(cckey.nelea-1, cckey.neleb-1);

  const bool do_b  = sigma->contains_block(bETkey);
  const bool do_bb = sigma->contains_block(bbETkey);
  const bool do_ab = sigma->contains_block(abETkey);
  assert(do_b || do_bb || do_ab);

  const int nccstates = cc_sector->nstates();

  shared_ptr<RASBlockVectors> b_sector = do_b ? sigma->sector(bETkey) : nullptr;
  shared_ptr<const RASDeterminants> b_det = do_b ? b_sector->det() : sigma->space()->det(cc_sector->det()->nelea(), cc_sector->det()->neleb()+1);

  shared_ptr<RASBlockVectors> bb_sector = do_bb ? sigma->sector(bbETkey) : nullptr;
  shared_ptr<RASBlockVectors> tmp_bb = do_bb ? make_shared<RASBlockVectors>(bb_sector->det(), BlockInfo(bbETkey.nelea, bbETkey.neleb, nccstates)) : nullptr;

  shared_ptr<RASBlockVectors> ab_sector = do_ab ? sigma->sector(abETkey) : nullptr;
  shared_ptr<RASBlockVectors> tmp_ab = do_ab ? make_shared<RASBlockVectors>(ab_sector->det(), BlockInfo(abETkey.nelea, abETkey.neleb, nccstates)) : nullptr;

  const BlockInfo bstate(bETkey.nelea, bETkey.neleb, nccstates);
  RASBlockVectors sector_r(b_det, BlockInfo(bETkey.nelea, bETkey.neleb, nccstates));

  const int phase = (1 - (((cc_sector->det()->nelea()+cc_sector->det()->neleb())%2) << 1));

#ifdef HAVE_MPI_H
  int mpi_counter = 0;
#endif

  for (int r = 0; r < rnorb; ++r) {
    sector_r.zero();
    apply(1.0, *cc_sector, sector_r, {GammaSQ::CreateBeta}, {r});
    if (do_b) {
#ifdef HAVE_MPI_H
      if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
        shared_ptr<const BlockSparseMatrix> Sr = blockops->S_b(bETkey, r);
        mat_block_multiply(false, false, phase, sector_r, *Sr, 1.0, *b_sector);
#ifdef HAVE_MPI_H
      }
#endif
    }

    if (do_bb) {
      for (int s = 0; s < r; ++s) {
#ifdef HAVE_MPI_H
        if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
          tmp_bb->zero();
          apply(1.0, sector_r, *tmp_bb, {GammaSQ::CreateBeta}, {s});

          shared_ptr<const BlockSparseMatrix> Prs = blockops->P_bb(cckey, s, r);
          mat_block_multiply(false, true, 2.0, *tmp_bb, *Prs, 1.0, *bb_sector);
#ifdef HAVE_MPI_H
        }
#endif
      }
    }

    if (do_ab) {
      for (int s = 0; s < rnorb; ++s) {
#ifdef HAVE_MPI_H
        if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
          tmp_ab->zero();
          apply(1.0, sector_r, *tmp_ab, {GammaSQ::CreateAlpha}, {s});

          shared_ptr<const BlockSparseMatrix> Prs = blockops->P_ab(cckey, s, r);
          mat_block_multiply(false, true, 1.0, *tmp_ab, *Prs, 1.0, *ab_sector);
#ifdef HAVE_MPI_H
        }
#endif
      }
    }
  }
}

void FormSigmaProdRAS::aHT_branch(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops) const {
  ApplyOperator apply;
  const int rnorb = cc_sector->det()->norb();

  // S_alpha
  const BlockKey cckey = cc_sector->left_state().key();

  const BlockKey aHTkey(cckey.nelea+1, cckey.neleb);
  const BlockKey aaHTkey(cckey.nelea+2, cckey.neleb);

  const bool do_aHT  = sigma->contains_block(aHTkey);
  const bool do_aaHT = sigma->contains_block(aaHTkey);
  // not sure how you could do a aa but not a a, but that's a different problem
  assert(do_aHT || do_aaHT);

  const int nccstates = cc_sector->nstates();

  shared_ptr<RASBlockVectors> a_sector = do_aHT ? sigma->sector(aHTkey) : nullptr;
  shared_ptr<const RASDeterminants> a_det = do_aHT ? a_sector->det() : sigma->space()->det(cc_sector->det()->nelea()-1,cc_sector->det()->neleb());

  shared_ptr<RASBlockVectors> aa_sector = do_aaHT ? sigma->sector(aaHTkey) : nullptr;
  shared_ptr<RASBlockVectors> tmp_aa = do_aaHT ? make_shared<RASBlockVectors>(aa_sector->det(), BlockInfo(aaHTkey.nelea, aaHTkey.neleb, nccstates)) : nullptr;

  RASBlockVectors sector_r(a_det, BlockInfo(aHTkey.nelea, aHTkey.neleb, nccstates));

  const int phase = (1 - (((sector_r.det()->nelea()+sector_r.det()->neleb())%2) << 1));

#ifdef HAVE_MPI_H
  int mpi_counter = 0;
#endif

  for (int r = 0; r < rnorb; ++r) {
    sector_r.zero();
    apply(1.0, *cc_sector, sector_r, {GammaSQ::AnnihilateAlpha}, {r});
    if (do_aHT) {
#ifdef HAVE_MPI_H
      if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
        shared_ptr<const BlockSparseMatrix> Sr = blockops->S_a(cckey, r);
        mat_block_multiply(false, true, phase, sector_r, *Sr, 1.0, *a_sector);
#ifdef HAVE_MPI_H
      }
#endif
    }

    if (do_aaHT) {
      for (int s = 0; s < r; ++s) {
#ifdef HAVE_MPI_H
        if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
          tmp_aa->zero();
          apply(1.0, sector_r, *tmp_aa, {GammaSQ::AnnihilateAlpha}, {s});

          shared_ptr<const BlockSparseMatrix> Prs = blockops->P_aa(aaHTkey, r, s);
          mat_block_multiply(false, false, 2.0, *tmp_aa, *Prs, 1.0, *aa_sector);
#ifdef HAVE_MPI_H
        }
#endif
      }
    }
  }
}

void FormSigmaProdRAS::bHT_branch(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops) const {
  ApplyOperator apply;
  const int rnorb = cc_sector->det()->norb();

  // S_beta
  const BlockKey cckey = cc_sector->left_state().key();

  const BlockKey bHTkey(cckey.nelea, cckey.neleb+1);
  const BlockKey bbHTkey(cckey.nelea, cckey.neleb+2);
  const BlockKey abHTkey(cckey.nelea+1, cckey.neleb+1);

  const bool do_bHT  = sigma->contains_block(bHTkey);
  const bool do_bbHT = sigma->contains_block(bbHTkey);
  const bool do_abHT = sigma->contains_block(abHTkey);
  assert(do_bHT || do_bbHT || do_abHT);

  const int nccstates = cc_sector->nstates();

  shared_ptr<RASBlockVectors> b_sector = do_bHT ? sigma->sector(bHTkey) : nullptr;
  shared_ptr<const RASDeterminants> b_det = do_bHT ? b_sector->det() : sigma->space()->det(cc_sector->det()->nelea(), cc_sector->det()->neleb()-1);

  RASBlockVectors sector_r(b_det, BlockInfo(bHTkey.nelea, bHTkey.neleb, nccstates));

  shared_ptr<RASBlockVectors> bb_sector = do_bbHT ? sigma->sector(bbHTkey) : nullptr;
  shared_ptr<RASBlockVectors> tmp_bb = do_bbHT ? make_shared<RASBlockVectors>(bb_sector->det(), BlockInfo(bbHTkey.nelea, bbHTkey.neleb, nccstates)) : nullptr;

  shared_ptr<RASBlockVectors> ab_sector = do_abHT ? sigma->sector(abHTkey) : nullptr;
  shared_ptr<RASBlockVectors> tmp_ab = do_abHT ? make_shared<RASBlockVectors>(ab_sector->det(), BlockInfo(abHTkey.nelea, abHTkey.neleb, nccstates)) : nullptr;

  const int phase = (1 - (((sector_r.det()->nelea()+sector_r.det()->neleb())%2) << 1));

#ifdef HAVE_MPI_H
  int mpi_counter = 0;
#endif

  for (int r = 0; r < rnorb; ++r) {
    sector_r.zero();
    apply(1.0, *cc_sector, sector_r, {GammaSQ::AnnihilateBeta}, {r});
    if (do_bHT) {
#ifdef HAVE_MPI_H
      if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
        shared_ptr<const BlockSparseMatrix> Sr = blockops->S_b(cckey, r);
        mat_block_multiply(false, true, phase, sector_r, *Sr, 1.0, *b_sector);
#ifdef HAVE_MPI_H
      }
#endif
    }

    if (do_bbHT) {
      for (int s = 0; s < r; ++s) {
#ifdef HAVE_MPI_H
        if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
          tmp_bb->zero();
          apply(1.0, sector_r, *tmp_bb, {GammaSQ::AnnihilateBeta}, {s});

          shared_ptr<const BlockSparseMatrix> Prs = blockops->P_bb(bbHTkey, r, s);
          mat_block_multiply(false, false, 2.0, *tmp_bb, *Prs, 1.0, *bb_sector);
#ifdef HAVE_MPI_H
        }
#endif
      }
    }

    if (do_abHT) {
      for (int s = 0; s < rnorb; ++s) {
#ifdef HAVE_MPI_H
        if (mpi_counter++ % mpi__->size() == mpi__->rank()) {
#endif
          tmp_ab->zero();
          apply(1.0, sector_r, *tmp_ab, {GammaSQ::AnnihilateAlpha}, {s});

          shared_ptr<const BlockSparseMatrix> Prs = blockops->P_ab(abHTkey, s, r);
          mat_block_multiply(false, false, -1.0, *tmp_ab, *Prs, 1.0, *ab_sector);
#ifdef HAVE_MPI_H
        }
#endif
      }
    }
  }
}


void FormSigmaProdRAS::aexc_branch(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops) const {
  ApplyOperator apply;
  const int rnorb = cc_sector->det()->norb();

  const BlockKey cckey = cc_sector->left_state().key();
  RASBlockVectors sector_rs(cc_sector->det(), cc_sector->left_state());

  shared_ptr<RASBlockVectors> sigma_sector = sigma->sector(cckey);

  for (int r = 0; r < rnorb; ++r) {
    for (int s = 0; s < rnorb; ++s) {
      // apply (r^dagger s)_alpha to the Civecs
#ifdef HAVE_MPI_H
      if ((r + s*rnorb) % mpi__->size() == mpi__->rank()) {
#endif
        sector_rs.zero();
        apply(1.0, *cc_sector, sector_rs, {GammaSQ::CreateAlpha,GammaSQ::AnnihilateAlpha}, {r,s});

        shared_ptr<const BlockSparseMatrix> Qrs = blockops->Q_aa(cckey, r, s);
        mat_block_multiply(false, true, 1.0, sector_rs, *Qrs, 1.0, *sigma_sector);
#ifdef HAVE_MPI_H
      }
#endif
    }
  }
}

void FormSigmaProdRAS::bexc_branch(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops) const {
  ApplyOperator apply;
  const int rnorb = cc_sector->det()->norb();

  const BlockKey cckey = cc_sector->left_state().key();
  RASBlockVectors sector_rs(cc_sector->det(), cc_sector->left_state());

  shared_ptr<RASBlockVectors> sigma_sector = sigma->sector(cckey);

  for (int r = 0; r < rnorb; ++r) {
    for (int s = 0; s < rnorb; ++s) {
#ifdef HAVE_MPI_H
      if ((r + s*rnorb) % mpi__->size() == mpi__->rank()) {
#endif
        // apply (r^dagger s)_beta to the Civecs
        sector_rs.zero();
        apply(1.0, *cc_sector, sector_rs, {GammaSQ::CreateBeta,GammaSQ::AnnihilateBeta}, {r,s});

        shared_ptr<const BlockSparseMatrix> Qrs = blockops->Q_bb(cckey, r, s);
        mat_block_multiply(false, true, 1.0, sector_rs, *Qrs, 1.0, *sigma_sector);
#ifdef HAVE_MPI_H
      }
#endif
    }
  }

}

void FormSigmaProdRAS::abflip_branch(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops) const {
  ApplyOperator apply;
  const int rnorb = cc_sector->det()->norb();

  const int nccstates = cc_sector->mdim();
  const BlockKey cckey = cc_sector->left_state().key();
  const BlockKey flipkey(cckey.nelea-1, cckey.neleb+1);

  assert(sigma->contains_block(flipkey));

  shared_ptr<const RASDeterminants> flipdet = sigma->sector(flipkey)->det();
  shared_ptr<RASBlockVectors> sigma_sector = sigma->sector(flipkey);

  RASBlockVectors sector_rs(flipdet, BlockInfo(flipkey.nelea, flipkey.neleb, nccstates));
  for (int r = 0; r < rnorb; ++r) {
    for (int s = 0; s < rnorb; ++s) {
      // apply (r^dagger_alpha s_beta) to the civec
#ifdef HAVE_MPI_H
      if ((r + s*rnorb) % mpi__->size() == mpi__->rank()) {
#endif
        sector_rs.zero();
        apply(1.0, *cc_sector, sector_rs, {GammaSQ::CreateAlpha,GammaSQ::AnnihilateBeta}, {r, s});

        shared_ptr<const BlockSparseMatrix> Qrs = blockops->Q_ab(cckey, r, s);
        mat_block_multiply(false, true, 1.0, sector_rs, *Qrs, 1.0, *sigma_sector);
#ifdef HAVE_MPI_H
      }
#endif
    }
  }
}

void FormSigmaProdRAS::baflip_branch(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops) const {
  ApplyOperator apply;
  const int rnorb = cc_sector->det()->norb();

  const int nccstates = cc_sector->mdim();
  const BlockKey cckey = cc_sector->left_state().key();
  const BlockKey flipkey(cckey.nelea+1, cckey.neleb-1);
  assert(sigma->contains_block(flipkey));

  shared_ptr<const RASDeterminants> flipdet = sigma->sector(flipkey)->det();
  shared_ptr<RASBlockVectors> sigma_sector = sigma->sector(flipkey);

  RASBlockVectors sector_rs(flipdet, BlockInfo(flipkey.nelea, flipkey.neleb, nccstates));
  for (int r = 0; r < rnorb; ++r) {
    for (int s = 0; s < rnorb; ++s) {
#ifdef HAVE_MPI_H
      if ((r + s*rnorb) % mpi__->size() == mpi__->rank()) {
#endif
        // apply (r^dagger_beta s_alpha) to the civec
        sector_rs.zero();
        apply(1.0, *cc_sector, sector_rs, {GammaSQ::CreateBeta,GammaSQ::AnnihilateAlpha}, {r, s});

        shared_ptr<const BlockSparseMatrix> Qrs = blockops->Q_ab(flipkey, s, r);
        mat_block_multiply(false, false, 1.0, sector_rs, *Qrs, 1.0, *sigma_sector);
#ifdef HAVE_MPI_H
      }
#endif
    }
  }
}

void FormSigmaProdRAS::compute_sigma_3aET(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops, shared_ptr<DimerJop> jop) const {
  const BlockKey aETkey(cc_sector->left_state().nelea-1, cc_sector->left_state().neleb);
  assert(sigma->contains_block(aETkey));
  shared_ptr<RASBlockVectors> sigma_sector = sigma->sector(aETkey);
  const int nccstates = cc_sector->mdim();
  const BlockInfo tmpinfo(aETkey.nelea, aETkey.neleb, nccstates);
  RASBlockVectors tmp_sector(sigma_sector->det(), tmpinfo);

  const int lnorb = jop->monomer_jop<1>()->nocc();
  const int rnorb = jop->monomer_jop<0>()->nocc();

  shared_ptr<const Matrix> J = jop->coulomb_matrix<0,1,0,0>();
  const int phase = (1 - (((cc_sector->det()->nelea()+cc_sector->det()->neleb())%2) << 1));

  auto sparseij = make_shared<Sparse_IJ>(cc_sector->det()->stringspaceb(), sigma_sector->det()->stringspaceb());
  auto phik = make_shared<PhiKLists>(cc_sector->det()->stringspacea(), sigma_sector->det()->stringspacea());

  for (int p = 0; p < lnorb; ++p ) {
#ifdef HAVE_MPI_H
    if (p % mpi__->size() == mpi__->rank()) {
#endif
      tmp_sector.zero();
      auto Jp = make_shared<btas::Tensor3<double>>(rnorb, rnorb, rnorb);
      copy_n(J->element_ptr(0, p), rnorb*rnorb*rnorb, Jp->data());
      for (int ist = 0; ist < nccstates; ++ist) {
        resolve_S_adag_adag_a(cc_sector->civec(ist), tmp_sector.civec(ist), Jp);
        resolve_S_abb(cc_sector->civec(ist), tmp_sector.civec(ist), Jp, phik, sparseij);
      }

      shared_ptr<const BlockSparseMatrix> gamma_a = blockops->gamma_a(aETkey, p);
      mat_block_multiply(false, false, phase, tmp_sector, *gamma_a, 1.0, *sigma_sector);
#ifdef HAVE_MPI_H
    }
#endif
  }
}

void FormSigmaProdRAS::compute_sigma_3aHT(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops, shared_ptr<DimerJop> jop) const {
  const BlockKey aHTkey(cc_sector->left_state().nelea+1, cc_sector->left_state().neleb);
  assert(sigma->contains_block(aHTkey));
  shared_ptr<RASBlockVectors> sigma_sector = sigma->sector(aHTkey);
  const int nccstates = cc_sector->mdim();
  const BlockInfo tmpinfo(aHTkey.nelea, aHTkey.neleb, nccstates);
  RASBlockVectors tmp_sector(sigma_sector->det(), tmpinfo);

  const int lnorb = jop->monomer_jop<1>()->nocc();
  const int rnorb = jop->monomer_jop<0>()->nocc();

  shared_ptr<const Matrix> J = jop->coulomb_matrix<0,1,0,0>();

  const int phase = (1 - (((tmp_sector.det()->nelea()+tmp_sector.det()->neleb())%2) << 1));

  auto sparseij = make_shared<Sparse_IJ>(cc_sector->det()->stringspaceb(), sigma_sector->det()->stringspaceb());
  auto phik = make_shared<PhiKLists>(cc_sector->det()->stringspacea(), sigma_sector->det()->stringspacea());

  for (int p = 0; p < lnorb; ++p ) {
#ifdef HAVE_MPI_H
    if (p % mpi__->size() == mpi__->rank()) {
#endif
      tmp_sector.zero();
      auto Jp = make_shared<btas::Tensor3<double>>(rnorb, rnorb, rnorb);
      copy_n(J->element_ptr(0, p), rnorb*rnorb*rnorb, Jp->data());
      for (int ist = 0; ist < nccstates; ++ist) {
        resolve_S_adag_a_a(cc_sector->civec(ist), tmp_sector.civec(ist), Jp);
        resolve_S_abb(cc_sector->civec(ist), tmp_sector.civec(ist), Jp, phik, sparseij);
      }

      shared_ptr<const BlockSparseMatrix> gamma_a = blockops->gamma_a(cc_sector->left_state(), p);
      mat_block_multiply(false, true, phase, tmp_sector, *gamma_a, 1.0, *sigma_sector);
#ifdef HAVE_MPI_H
    }
#endif
  }
}

void FormSigmaProdRAS::compute_sigma_3bET(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops,  shared_ptr<DimerJop> jop) const {
  const BlockKey bETkey(cc_sector->left_state().nelea, cc_sector->left_state().neleb-1);
  assert(sigma->contains_block(bETkey));
  shared_ptr<RASBlockVectors> sigma_sector = sigma->sector(bETkey);
  const int nccstates = cc_sector->mdim();
  const BlockInfo tmpinfo(bETkey.nelea, bETkey.neleb, nccstates);

  RASBlockVectors sigma_trans(sigma_sector->det()->transpose(), sigma_sector->left_state());
  RASBlockVectors tmp_sector(sigma_trans.det(), tmpinfo);

  const int lnorb = jop->monomer_jop<1>()->nocc();
  const int rnorb = jop->monomer_jop<0>()->nocc();

  shared_ptr<const Matrix> J = jop->coulomb_matrix<0,1,0,0>();

  const int phase = (1 - (((cc_sector->det()->nelea()+cc_sector->det()->neleb())%2) << 1));

  shared_ptr<RASDeterminants> trans_det = cc_sector->det()->transpose();
  vector<shared_ptr<RASCivec>> cc_trans;
  for (int icc = 0; icc < nccstates; ++icc)
    cc_trans.push_back(cc_sector->civec(icc).transpose(trans_det));

  auto sparseij = make_shared<Sparse_IJ>(cc_trans.front()->det()->stringspaceb(), sigma_trans.det()->stringspaceb());
  auto phik = make_shared<PhiKLists>(cc_trans.front()->det()->stringspacea(), sigma_trans.det()->stringspacea());

  for (int p = 0; p < lnorb; ++p) {
#ifdef HAVE_MPI_H
    if (p % mpi__->size() == mpi__->rank()) {
#endif
      tmp_sector.zero();
      auto Jp = make_shared<btas::Tensor3<double>>(rnorb, rnorb, rnorb);
      copy_n(J->element_ptr(0, p), rnorb*rnorb*rnorb, Jp->data());
      for (int ist = 0; ist < nccstates; ++ist) {
        resolve_S_adag_adag_a(*cc_trans[ist], tmp_sector.civec(ist), Jp);
        resolve_S_abb(*cc_trans[ist], tmp_sector.civec(ist), Jp, phik, sparseij);
      }

      shared_ptr<const BlockSparseMatrix> gamma_b = blockops->gamma_b(bETkey, p);
      mat_block_multiply(false, false, phase, tmp_sector, *gamma_b, 1.0, sigma_trans);
#ifdef HAVE_MPI_H
    }
#endif
  }

  for (int isg = 0; isg < sigma_sector->mdim(); ++isg)
    sigma_sector->civec(isg).ax_plus_y(1.0, *sigma_trans.civec(isg).transpose(sigma_sector->det()));
}


void FormSigmaProdRAS::compute_sigma_3bHT(shared_ptr<const RASBlockVectors> cc_sector, shared_ptr<ProductRASCivec> sigma, shared_ptr<const BlockOperators> blockops, shared_ptr<DimerJop> jop) const {
  const BlockKey bHTkey(cc_sector->left_state().nelea, cc_sector->left_state().neleb+1);
  assert(sigma->contains_block(bHTkey));
  shared_ptr<RASBlockVectors> sigma_sector = sigma->sector(bHTkey);
  const int nccstates = cc_sector->mdim();
  const BlockInfo tmpinfo(bHTkey.nelea, bHTkey.neleb, nccstates);

  RASBlockVectors sigma_trans(sigma_sector->det()->transpose(), sigma_sector->left_state());
  RASBlockVectors tmp_sector(sigma_trans.det(), tmpinfo);

  const int lnorb = jop->monomer_jop<1>()->nocc();
  const int rnorb = jop->monomer_jop<0>()->nocc();

  shared_ptr<const Matrix> J = jop->coulomb_matrix<0,1,0,0>();

  const int phase = (1 - (((tmp_sector.det()->nelea()+tmp_sector.det()->neleb())%2) << 1));

  shared_ptr<RASDeterminants> trans_det = cc_sector->det()->transpose();
  vector<shared_ptr<RASCivec>> cc_trans;
  for (int icc = 0; icc < nccstates; ++icc)
    cc_trans.push_back(cc_sector->civec(icc).transpose(trans_det));

  auto sparseij = make_shared<Sparse_IJ>(cc_trans.front()->det()->stringspaceb(), sigma_trans.det()->stringspaceb());
  auto phik = make_shared<PhiKLists>(cc_trans.front()->det()->stringspacea(), sigma_trans.det()->stringspacea());

  for (int p = 0; p < lnorb; ++p ) {
#ifdef HAVE_MPI_H
    if (p % mpi__->size() == mpi__->rank()) {
#endif
      tmp_sector.zero();
      auto Jp = make_shared<btas::Tensor3<double>>(rnorb, rnorb, rnorb);
      copy_n(J->element_ptr(0, p), rnorb*rnorb*rnorb, Jp->data());
      for (int ist = 0; ist < nccstates; ++ist) {
        resolve_S_adag_a_a(*cc_trans[ist], tmp_sector.civec(ist), Jp);
        resolve_S_abb(*cc_trans[ist], tmp_sector.civec(ist), Jp, phik, sparseij);
      }

      shared_ptr<const BlockSparseMatrix> gamma_b = blockops->gamma_b(cc_sector->left_state(), p);
      mat_block_multiply(false, true, phase, tmp_sector, *gamma_b, 1.0, sigma_trans);
#ifdef HAVE_MPI_H
    }
#endif
  }

  for (int isg = 0; isg < sigma_sector->mdim(); ++isg)
    sigma_sector->civec(isg).ax_plus_y(1.0, *sigma_trans.civec(isg).transpose(sigma_sector->det()));
}


// resolves (S_p)_alpha = \sum_{ijk} (pk|ij) (k^+)_alpha i^+_alpha j_alpha
void FormSigmaProdRAS::resolve_S_adag_adag_a(const RASCivecView cc, RASCivecView sigma, shared_ptr<btas::Tensor3<double>> Jp) const {
  shared_ptr<const RASDeterminants> sdet = cc.det();
  shared_ptr<const RASDeterminants> tdet = sigma.det();

  const int norb = sdet->norb();
  assert(norb == tdet->norb());

  const size_t sla = sdet->lena();

  // k^+_alpha i^+_alpha j_alpha portion. the way easy part
  Matrix F(sla, batchsize_, true);
  for (auto& targetspace : *tdet->stringspacea()) {
    const int nbatches = (targetspace->size()-1)/batchsize_ + 1;
    for (int batch = 0; batch < nbatches; ++batch) {
      const size_t batchstart = batch*batchsize_;
      const size_t batchlength = min(static_cast<size_t>(batchsize_), targetspace->size() - batchstart);

      F.zero();

      // first build an F
      for (size_t ia = 0; ia < batchlength; ++ia) {
        double* const fdata = F.element_ptr(0, ia);
        const bitset<nbit__> tabit = targetspace->strings(ia + batchstart);
        for (int k = 0; k < norb; ++k) {
          if (!tabit[k]) continue;
          const bitset<nbit__> tmpbit = tabit ^ (bitset<nbit__>(1) << k);
          const int kphase = sign(tmpbit, k);
          for (int i = 0; i < k; ++i) {
            if (!tmpbit[i]) continue;
            const bitset<nbit__> tmp2bit = tmpbit ^ (bitset<nbit__>(1) << i);
            for (int j = 0; j < norb; ++j) {
              if (tmp2bit[j]) continue;
              const bitset<nbit__> sbit = tmp2bit ^ (bitset<nbit__>(1) << j);
              if (sdet->allowed(sbit)) {
                const int ijphase = sign(sbit, i, j);
                const size_t source_ia = sdet->lexical_offset<0>(sbit);
                fdata[source_ia] += static_cast<double>(kphase * ijphase) * ((*Jp)(j, i, k) - (*Jp)(j, k, i));
              }
            }
          }
        }
      }

      // Sp(beta, alpha) += \sum_alpha' C(beta, alpha') * F(alpha', alpha)
      for (auto& ccblock : cc.blocks()) {
        if (!ccblock) continue;
        if (!tdet->allowed(targetspace, ccblock->stringsb())) continue;
        shared_ptr<RASBlock<double>> target_block = sigma.block(ccblock->stringsb(), targetspace);

        assert(ccblock->lenb() == target_block->lenb());
        dgemm_("N", "N", target_block->lenb(), batchlength, ccblock->lena(), 1.0, ccblock->data(), ccblock->lenb(),
                  F.element_ptr(ccblock->stringsa()->offset(), 0), F.ndim(), 1.0, target_block->data() + batchstart * target_block->lenb(), target_block->lenb());
      }
    }
  }
}

// resolves (S_p)_alpha^\dagger = \sum_{ijk} (pk|ij) (i^+)_alpha j_alpha k_alpha
void FormSigmaProdRAS::resolve_S_adag_a_a(const RASCivecView cc, RASCivecView sigma, shared_ptr<btas::Tensor3<double>> Jp) const {
  shared_ptr<const RASDeterminants> sdet = cc.det();
  shared_ptr<const RASDeterminants> tdet = sigma.det();

  const int norb = sdet->norb();
  assert(norb == tdet->norb());

  const size_t sla = sdet->lena();

  // i^+_alpha j^_alpha k_alpha portion. the way easy part
  Matrix F(sla, batchsize_, true);
  for (auto& targetspace : *tdet->stringspacea()) {
    const int nbatches = (targetspace->size()-1)/batchsize_ + 1;
    for (int batch = 0; batch < nbatches; ++batch) {
      const size_t batchstart = batch*batchsize_;
      const size_t batchlength = min(static_cast<size_t>(batchsize_), targetspace->size() - batchstart);

      F.zero();

      // first build an F
      const size_t offset = batchstart + targetspace->offset();
      for (size_t ia = 0; ia < batchlength; ++ia) {
        double* const fdata = F.element_ptr(0, ia);
        const bitset<nbit__> tbit = tdet->string_bits_a(ia+offset);
        for (int i = 0; i < norb; ++i) {
          if (!tbit[i]) continue;
          const bitset<nbit__> tmpbit = tbit ^ (bitset<nbit__>(1) << i);
          for (int j = 0; j < norb; ++j) {
            if (tmpbit[j]) continue;
            const int ijphase = sign(tbit, i, j);
            const bitset<nbit__> tmp2bit = tmpbit ^ (bitset<nbit__>(1) << j);
            for (int k = j+1; k < norb; ++k) {
              if (tmp2bit[k]) continue;
              const bitset<nbit__> sbit = tmp2bit ^ (bitset<nbit__>(1) << k);
              if (sdet->allowed(sbit)) {
                const int kphase = sign(sbit, k);
                const size_t source_ia = sdet->lexical_offset<0>(sbit);
                fdata[source_ia] += static_cast<double>(kphase * ijphase) * ((*Jp)(j, i, k) - (*Jp)(k, i, j));
              }
            }
          }
        }
      }

      // Sp(beta, alpha) += \sum_alpha' C(beta, alpha') * F(alpha', alpha)
      for (auto& ccblock : cc.blocks()) {
        if (!ccblock) continue;
        if (!tdet->allowed(targetspace, ccblock->stringsb())) continue;
        shared_ptr<RASBlock<double>> target_block = sigma.block(ccblock->stringsb(), targetspace);

        assert(ccblock->lenb() == target_block->lenb());
        dgemm_("N", "N", target_block->lenb(), batchlength, ccblock->lena(), 1.0, ccblock->data(), ccblock->lenb(),
                  F.element_ptr(ccblock->stringsa()->offset(), 0), F.ndim(), 1.0, target_block->data() + batchstart * target_block->lenb(), target_block->lenb());
      }
    }
  }
}

// computes \sum_{ijk} (k)_alpha i^+_beta j_beta (pk|ij)
// (k) can be creation or annihilation and is determined based on the electron count of sigma
void FormSigmaProdRAS::resolve_S_abb(const RASCivecView cc, RASCivecView sigma, shared_ptr<btas::Tensor3<double>> Jp, shared_ptr<PhiKLists> phik, shared_ptr<Sparse_IJ> sparseij) const {
  shared_ptr<const RASDeterminants> sdet = cc.det();
  shared_ptr<const RASDeterminants> tdet = sigma.det();

  assert(abs(sdet->nelea()-tdet->nelea())==1);
  assert(sdet->neleb()==tdet->neleb());

  const int norb = sdet->norb();
  assert(norb == tdet->norb());
  if (!sparseij) sparseij = make_shared<Sparse_IJ>(sdet->stringspaceb(), tdet->stringspaceb());
  if (!phik) phik = make_shared<PhiKLists>(sdet->stringspacea(), tdet->stringspacea());

  // k^?_alpha i^+_beta j_beta portion. the harder part
  for (int k = 0; k < norb; ++k) {
    for (auto& target_bspace : *tdet->stringspaceb()) {
      for (auto& source_aspace : *sdet->stringspacea()) {
        const vector<PhiKLists::PhiK>& full_phi = phik->data(k).at(source_aspace->tag());
        vector<PhiKLists::PhiK> reduced_RI;

        for (auto& i : full_phi) {
          const RASString* target_aspace = i.target_space;
          if (tdet->allowed(target_aspace->nholes(), target_bspace->nholes(), target_aspace->nparticles(), target_bspace->nparticles()))
            reduced_RI.push_back(i);
        }

        if (reduced_RI.empty()) continue;

        for (auto& source_block : cc.allowed_blocks<0>(source_aspace)) {
          auto source_bspace = source_block->stringsb();
          const size_t slb = source_bspace->size();
          auto reduced_cp = make_shared<Matrix>(slb, reduced_RI.size(), true);
          int current = 0;
          for (auto& i : reduced_RI)
            blas::ax_plus_y_n(i.sign, source_block->data() + slb*i.source, slb, reduced_cp->element_ptr(0,current++));

          // Now build an F matrix in sparse format
          shared_ptr<SparseMatrix> sparseF = sparseij->sparse_matrix(target_bspace->tag(), source_bspace->tag());

          if (sparseF) {
            sparseF->zero();
            for (auto& iter : sparseij->sparse_data(target_bspace->tag(), source_bspace->tag()))
              *iter.ptr += static_cast<double>(iter.sign) * (*Jp)(iter.j, iter.i, k);

            Matrix V(*sparseF * *reduced_cp);

            // scatter
            current = 0;
            for (auto& i : reduced_RI) {
              const RASString* target_aspace = i.target_space;
              shared_ptr<RASBlock<double>> target_block = sigma.block(target_aspace->nholes(), target_bspace->nholes(), target_aspace->nparticles(), target_bspace->nparticles());
              blas::ax_plus_y_n(1.0, V.element_ptr(0,current++), V.ndim(), target_block->data() + target_block->lenb()*i.target);
            }
          }
        }
      }
    }
  }
}
