//
// Author : Toru Shiozaki
// Date   : Jan 2012
//

#ifndef __NEWINT_SRC_SCF_FOCK_H
#define __NEWINT_SRC_SCF_FOCK_H

#include <iostream>
#include <memory>
#include <stdexcept>
#include <src/util/f77.h>
#include <src/rysint/eribatch.h>
#include <src/scf/fock_base.h>

template<int DF>
class Fock : public Fock_base {

  public:
    Fock(const std::shared_ptr<Geometry> a, const std::shared_ptr<Fock<DF> > b, const std::shared_ptr<Matrix1e> c, const std::vector<double>& d)
     : Fock_base(a,b,c,d) {
      fock_two_electron_part();
      fock_one_electron_part();
    };
    Fock(const std::shared_ptr<Geometry> a, const std::shared_ptr<Hcore> b) : Fock_base(a,b) {};
    ~Fock() {};

    void fock_two_electron_part();
};


template<int DF>
void Fock<DF>::fock_two_electron_part() {
  
  // for debug <- what did I mean by this?? TODO
  density_->symmetrize();

  const std::vector<std::shared_ptr<Atom> > atoms = geom_->atoms(); 
  std::vector<std::shared_ptr<Shell> > basis; 
  std::vector<int> offset;
  int cnt = 0;
  for (auto aiter = atoms.begin(); aiter != atoms.end(); ++aiter, ++cnt) {
    const std::vector<std::shared_ptr<Shell> > tmp = (*aiter)->shells();
    basis.insert(basis.end(), tmp.begin(), tmp.end());  
    const std::vector<int> tmpoff = geom_->offset(cnt); 
    offset.insert(offset.end(), tmpoff.begin(), tmpoff.end());
  }

  const int shift = sizeof(int) * 4; 
  const int size = basis.size();

  // first make max_density_change std::vector for each batch pair.
  const double* density_data = density_->data();
   
  std::vector<double> max_density_change(size * size);
  for (int i = 0; i != size; ++i) {
    const int ioffset = offset[i];
    const int isize = basis[i]->nbasis(); 
    for (int j = i; j != size; ++j) { 
      const int joffset = offset[j];
      const int jsize = basis[j]->nbasis();

      double cmax = 0.0;
      for (int ii = ioffset; ii != ioffset + isize; ++ii) {  
        const int iin = ii * nbasis_;
        for (int jj = joffset; jj != joffset + jsize; ++jj) {  
          cmax = std::max(cmax, ::fabs(density_data[iin + jj]));
        }
      }
      const int ij = i * size + j;
      const int ji = i * size + j;
      max_density_change[ij] = cmax;
      max_density_change[ji] = cmax;
    }
  } 

  ////////////////////////////////////////////
  // starting 2-e Fock matrix evaluation!
  ////////////////////////////////////////////
  if (DF == 0) {
    std::shared_ptr<Petite> plist = geom_->plist();; 
    const bool c1 = plist->nirrep() == 1;

    for (int i0 = 0; i0 != size; ++i0) {
      if (!plist->in_p1(i0)) continue;

      const std::shared_ptr<Shell>  b0 = basis[i0];
      const int b0offset = offset[i0]; 
      const int b0size = b0->nbasis();
      for (int i1 = i0; i1 != size; ++i1) {
        const unsigned int i01 = i0 *size + i1;
        if (!plist->in_p2(i01)) continue;

        const std::shared_ptr<Shell>  b1 = basis[i1];
        const int b1offset = offset[i1]; 
        const int b1size = b1->nbasis();

        const double density_change_01 = max_density_change[i01] * 4.0; 

        for (int i2 = i0; i2 != size; ++i2) {
          const std::shared_ptr<Shell>  b2 = basis[i2];
          const int b2offset = offset[i2]; 
          const int b2size = b2->nbasis();

          const double density_change_02 = max_density_change[i0 * size + i2];
          const double density_change_12 = max_density_change[i1 * size + i2];

          for (int i3 = i2; i3 != size; ++i3) {
            const unsigned int i23 = i2 * size + i3;
            if (i23 < i01) continue;
            int ijkl = plist->in_p4(i01, i23, i0, i1, i2, i3);
            if (ijkl == 0) continue;
    
            const double density_change_23 = max_density_change[i2 * size + i3] * 4.0;
            const double density_change_03 = max_density_change[i0 * size + i2];
            const double density_change_13 = max_density_change[i0 * size + i2];

            const bool eqli01i23 = (i01 == i23);

            const std::shared_ptr<Shell>  b3 = basis[i3];
            const int b3offset = offset[i3]; 
            const int b3size = b3->nbasis();

            const double mulfactor = std::max(std::max(std::max(density_change_01, density_change_02), 
                                             std::max(density_change_12, density_change_23)), 
                                             std::max(density_change_03, density_change_13));
            const double integral_bound = mulfactor * schwarz_[i01] * schwarz_[i23];
            const bool skip_schwarz = integral_bound < schwarz_thresh_;
            if (skip_schwarz) continue;

            std::vector<std::shared_ptr<Shell> > input;
            input.push_back(b3);
            input.push_back(b2);
            input.push_back(b1);
            input.push_back(b0);

            ERIBatch eribatch(input, mulfactor);
            eribatch.compute();
            const double* eridata = eribatch.data();

            assert((int)eribatch.data_size() == b0size * b1size * b2size * b3size);

            for (int j0 = b0offset; j0 != b0offset + b0size; ++j0) {  
              const int j0n = j0 * nbasis_;

              for (int j1 = b1offset; j1 != b1offset + b1size; ++j1) {  
                const unsigned int nj01 = (j0 << shift) + j1; 
                const bool skipj0j1 = (j0 > j1);
                if (skipj0j1) {
                  eridata += b2size * b3size;
                  continue;
                }

                const bool eqlj0j1 = (j0 == j1);
                const double scal01 = (eqlj0j1 ? 0.5 : 1.0) * static_cast<double>(ijkl);
                const int j1n = j1 * nbasis_;

                for (int j2 = b2offset; j2 != b2offset + b2size; ++j2) {  
                  const int maxj1j2 = std::max(j1, j2);
                  const int minj1j2 = std::min(j1, j2);
                  const int minj1j2n = minj1j2 * nbasis_;

                  const int maxj0j2 = std::max(j0, j2);
                  const int minj0j2 = std::min(j0, j2);
                  const int minj0j2n = minj0j2 * nbasis_;
                  const int j2n = j2 * nbasis_;

                  for (int j3 = b3offset; j3 != b3offset + b3size; ++j3, ++eridata) {  
                    const bool skipj2j3 = (j2 > j3);
                    const unsigned int nj23 = (j2 << shift) + j3;
                    const bool skipj01j23 = (nj01 > nj23) && eqli01i23;

                    if (skipj2j3 || skipj01j23) continue;

                    const int maxj1j3 = std::max(j1, j3); 
                    const int minj1j3 = std::min(j1, j3); 

                    double intval = *eridata * scal01 * (j2 == j3 ? 0.5 : 1.0) * (nj01 == nj23 ? 0.5 : 1.0);
                    const double intval4 = 4.0 * intval;

                    data_[j0n + j1] += density_data[j2n + j3] * intval4;
                    data_[j2n + j3] += density_data[j0n + j1] * intval4;
                    data_[j0n + j3] -= density_data[j1n + j2] * intval;
                    data_[minj1j2n + maxj1j2] -= density_data[j0n + j3] * intval;
                    data_[minj0j2n + maxj0j2] -= density_data[j1n + j3] * intval;
                    data_[minj1j3 * nbasis_ + maxj1j3] -= density_data[j0n + j2] * intval;
                  }             
                }
              }
            }

          }
        }
      }
    }
  //////////////////////////////////////////////////////////////////////////////////////////////////
  } else if (DF == 1) {

    // some info for auxiliary (i.e., DF) basis set
    const std::vector<std::shared_ptr<Atom> > aux_atoms = geom_->aux_atoms(); 
    std::vector<std::shared_ptr<Shell> > aux_basis; 
    std::vector<int> aux_offset;
    int cnt = 0;
    for (auto aiter = aux_atoms.begin(); aiter != aux_atoms.end(); ++aiter, ++cnt) {
      const std::vector<std::shared_ptr<Shell> > tmp = (*aiter)->shells();
      aux_basis.insert(aux_basis.end(), tmp.begin(), tmp.end());  
      const std::vector<int> tmpoff = geom_->aux_offset(cnt); 
      aux_offset.insert(aux_offset.end(), tmpoff.begin(), tmpoff.end());
    }
    const int aux_size = aux_basis.size();

    const std::shared_ptr<Shell> b3(new Shell(basis.front()->spherical()));

    for (int i0 = 0; i0 != size; ++i0) {
      const std::shared_ptr<Shell>  b0 = basis[i0];
      const int b0offset = offset[i0]; 
      const int b0size = b0->nbasis();
      for (int i1 = i0; i1 != size; ++i1) {
//      const unsigned int i01 = i0 *size + i1;
        const std::shared_ptr<Shell>  b1 = basis[i1];
        const int b1offset = offset[i1]; 
        const int b1size = b1->nbasis();
// TODO haven't though about density screening...
//      const double density_change_01 = max_density_change[i01] * 4.0; 
        for (int i2 = 0; i2 != aux_size; ++i2) {
          const std::shared_ptr<Shell>  b2 = aux_basis[i2];
          const int b2offset = aux_offset[i2]; 
          const int b2size = b2->nbasis();

          std::vector<std::shared_ptr<Shell> > input;
          input.push_back(b3);
          input.push_back(b2);
          input.push_back(b1);
          input.push_back(b0);

          // TODO primitive screening is off
          ERIBatch eribatch(input, 1.0);
          eribatch.compute();
          const double* eridata = eribatch.data();
          std::cout << ddot_(b0size * b1size * b2size, eridata, 1, eridata, 1) << std::endl;

          assert((int)eribatch.data_size() == b0size * b1size * b2size);
        }
      }
    }
  }
  std::cout << "somehow came out" << std::endl;
  throw;
};


#endif
