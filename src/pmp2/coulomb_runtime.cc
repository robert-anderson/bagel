/*
 * coulomb_builder.cc
 *
 *  Created on: Nov 4, 2009
 *      Author: shiozaki
 */

#include <src/pmp2/pmp2.h>
#include <src/pscf/poverlap.h>
#include <src/pscf/ptildex.h>
#include <src/pscf/phcore.h>
#include <src/pscf/pcoeff.h>
#include <src/util/pcompcabsfile.h>

using namespace std;
using namespace boost;

typedef shared_ptr<PMatrix1e> RefMatrix;
typedef shared_ptr<PGeometry> RefGeom;
typedef shared_ptr<PHcore> RefHcore;
typedef shared_ptr<PCoeff> RefCoeff;

RefMatrix PMP2::coulomb_runtime() const {

  const double gamma = geom_->gamma();
  RefMatrix density(new PMatrix1e(coeff_->form_density_rhf()));
  const complex<double>* den = density->data()->front();

  RefMatrix coulomb_real_space(new PMatrix1e(union_geom_));
  complex<double>* data = coulomb_real_space->data()->front();

  const int s = geom_->S();
  const int l = geom_->L();

  /// oooo loop starts here.
  {
    const size_t allocsize_o = eri_obs_->max_num_int();
    double* diskdata_o = new double[allocsize_o];

    long file_position = 0l;
    size_t mcnt = 0lu;
    for (int m1 = -s; m1 <= s; ++m1) {
      for (int m2 = 0; m2 <= l; ++m2) { // use bra-ket symmetry!!!
        for (int m3 = m2 - s; m3 <= m2 + s; ++m3, ++mcnt) {

          const int k = geom_->K();
          const size_t b = density->blocksize();
          const size_t qb = coulomb_real_space->blocksize();

          const int n = geom_->nbasis();
          const int q = union_geom_->nbasis();

          // for density.
          const int m2___m3___k____b = (m2 - m3 + k) * b;
          const int _____m1___k____b = (   - m1 + k) * b;

          {
            const int m1________k____qb = (m1      + k) * qb;
            const int m3___m2___k____qb = (m3 - m2 + k) * qb;

            eri_obs_->get_block(file_position, eri_obs_->num_int_each(mcnt), diskdata_o);
            file_position += eri_obs_->num_int_each(mcnt);
            const double* cdata = diskdata_o;

            const int size = eri_obs_->basissize(); // number of shells
            for (int i0 = 0; i0 != size; ++i0) {
              const int b0offset = eri_obs_->offset(i0);
              const int b0size = eri_obs_->nbasis(i0);

              for (int i1 = 0; i1 != size; ++i1) {
                const int b1offset = eri_obs_->offset(i1);
                const int b1size = eri_obs_->nbasis(i1);

                for (int i2 = 0; i2 != size; ++i2) {
                  const int b2offset = eri_obs_->offset(i2);
                  const int b2size = eri_obs_->nbasis(i2);

                  for (int i3 = 0; i3 != size; ++i3) {
                    const int b3offset = eri_obs_->offset(i3);
                    const int b3size = eri_obs_->nbasis(i3);

                    const double integral_bound = eri_obs_->schwarz(((m1      + k) * size + i0) * size + i1)
                                                * eri_obs_->schwarz(((m3 - m2 + k) * size + i2) * size + i3);
                    const bool skip_schwarz = integral_bound < SCHWARZ_THRESH;
                    if (skip_schwarz) continue;

                    if (m2 != 0) {
                      for (int j0 = b0offset, j0q = b0offset * q; j0 != b0offset + b0size; ++j0, j0q += q) { // center unit cell
                        for (int j1 = b1offset, j1n = b1offset * n; j1 != b1offset + b1size; ++j1, j1n += n) {
                          for (int j2 = b2offset, j2q = b2offset * q; j2 != b2offset + b2size; ++j2, j2q += q) {
                            for (int j3 = b3offset, j3n = b3offset * n; j3 != b3offset + b3size; ++j3, j3n += n, ++cdata) {
                              const double integral2 = *cdata + *cdata;
                              data[m1________k____qb + j0q + j1] += den[m2___m3___k____b + j3n + j2] * integral2;
                              data[m3___m2___k____qb + j2q + j3] += den[_____m1___k____b + j1n + j0] * integral2;
                            }
                          }
                        }
                      }
                    } else {
                      for (int j0 = b0offset, j0q = b0offset * q; j0 != b0offset + b0size; ++j0, j0q += q) { // center unit cell
                        for (int j1 = b1offset                    ; j1 != b1offset + b1size; ++j1          ) {
                          for (int j2 = b2offset                    ; j2 != b2offset + b2size; ++j2          ) {
                            for (int j3 = b3offset, j3n = b3offset * n; j3 != b3offset + b3size; ++j3, j3n += n, ++cdata) {
                              const double integral2 = *cdata + *cdata;
                              data[m1________k____qb + j0q + j1] += den[m2___m3___k____b + j3n + j2] * integral2;
                            }
                          }
                        }
                      }
                    }

                  }
                }
              }
            }
          }

        }
      }
    }
    delete[] diskdata_o;
  }
  /// oooo loop ends.

  /// ooco loop starts here.
  {
    const size_t allocsize_cs = eri_cabs_->max_num_int();
    double* diskdata_cs = new double[allocsize_cs];

    long file_position = 0l;
    size_t mcnt = 0lu;
    for (int m1 = -s; m1 <= s; ++m1) {
      for (int m2 = -l; m2 <= l; ++m2) { // *NOT* using bra-ket symmetry!!!
        for (int m3 = m2 - s; m3 <= m2 + s; ++m3, ++mcnt) {

          const int k = geom_->K();
          const size_t b = density->blocksize();
          const size_t qb = coulomb_real_space->blocksize();

          const int n = geom_->nbasis();
          const int q = union_geom_->nbasis();

          // for density.
          const int m2___m3___k____b = (m2 - m3 + k) * b;
          const int _____m1___k____b = (   - m1 + k) * b;

          {
            const int m1________k____qb = (m1      + k) * qb;
            const int m3___m2___k____qb = (m3 - m2 + k) * qb;

            eri_cabs_->get_block(file_position, eri_cabs_->num_int_each(mcnt), diskdata_cs);
            file_position += eri_cabs_->num_int_each(mcnt);
            const double* cdata = diskdata_cs;

            const int size_i = eri_cabs_->size_i();
            for (int i0 = 0; i0 != size_i; ++i0) { // OBS
              const int b0offset = eri_cabs_->offset_i(i0);
              const int b0size = eri_cabs_->basis_i(i0)->nbasis();

              const int size_a = eri_cabs_->size_a();
              for (int i1 = 0; i1 != size_a; ++i1) { // CABS!!!
                const int b1offset = eri_cabs_->offset_a(i1) + geom_->nbasis();
                const int b1size = eri_cabs_->basis_a(i1)->nbasis();

                const int size_j = eri_cabs_->size_j();
                for (int i2 = 0; i2 != size_j; ++i2) { // OBS
                  const int b2offset = eri_cabs_->offset_j(i2);
                  const int b2size = eri_cabs_->basis_j(i2)->nbasis();

                  const int size_b = eri_cabs_->size_b();
                  for (int i3 = 0; i3 != size_b; ++i3) { // OBS
                    const int b3offset = eri_cabs_->offset_b(i3);
                    const int b3size = eri_cabs_->basis_b(i3)->nbasis();

                    const double integral_bound = eri_cabs_->schwarz_ia(((m1      + k) * size_i + i0) * size_a + i1)
                                                * eri_cabs_->schwarz_jb(((m3 - m2 + k) * size_j + i2) * size_b + i3);
                    const bool skip_schwarz = integral_bound < SCHWARZ_THRESH;
                    if (skip_schwarz) continue;

                    for (int j0 = b0offset, j0q = b0offset * q; j0 != b0offset + b0size; ++j0, j0q += q) { // center unit cell
                      for (int j1 = b1offset                    ; j1 != b1offset + b1size; ++j1          ) {
                        for (int j2 = b2offset                    ; j2 != b2offset + b2size; ++j2          ) {
                          for (int j3 = b3offset, j3n = b3offset * n; j3 != b3offset + b3size; ++j3, j3n += n, ++cdata) {
                            const double integral2 = *cdata + *cdata;
                            data[m1________k____qb + j0q + j1] += den[m2___m3___k____b + j3n + j2] * integral2;
                          }
                        }
                      }
                    }

                  }
                }
              }
            }
          }

        }
      }
    }
    delete[] diskdata_cs;
  }
  /// ooco loop ends.

  /// cooo loop starts here.
  {
    shared_ptr<PCompCABSFile<ERIBatch> >
      eri_cabs_d(new PCompCABSFile<ERIBatch>(geom_, gamma, true, false, false, false, false, "ERI CABS(i)"));
    eri_cabs_d->store_integrals();
    eri_cabs_d->reopen_with_inout();
    const size_t allocsize_cs = eri_cabs_d->max_num_int();
    double* diskdata_cs = new double[allocsize_cs];

    long file_position = 0l;
    size_t mcnt = 0lu;
    for (int m1 = -s; m1 <= s; ++m1) {
      for (int m2 = -l; m2 <= l; ++m2) { // *NOT* using bra-ket symmetry!!!
        for (int m3 = m2 - s; m3 <= m2 + s; ++m3, ++mcnt) {

          const int k = geom_->K();
          const size_t b = density->blocksize();
          const size_t qb = coulomb_real_space->blocksize();

          const int n = geom_->nbasis();
          const int q = union_geom_->nbasis();

          // for density.
          const int m2___m3___k____b = (m2 - m3 + k) * b;
          const int _____m1___k____b = (   - m1 + k) * b;

          {
            const int m1________k____qb = (m1      + k) * qb;
            const int m3___m2___k____qb = (m3 - m2 + k) * qb;

            eri_cabs_d->get_block(file_position, eri_cabs_d->num_int_each(mcnt), diskdata_cs);
            file_position += eri_cabs_d->num_int_each(mcnt);
            const double* cdata = diskdata_cs;

            const int size_i = eri_cabs_d->size_i();
            for (int i0 = 0; i0 != size_i; ++i0) { // CABS!!!!
              const int b0offset = eri_cabs_d->offset_i(i0) + geom_->nbasis();
              const int b0size = eri_cabs_d->basis_i(i0)->nbasis();

              const int size_a = eri_cabs_d->size_a();
              for (int i1 = 0; i1 != size_a; ++i1) { // OBS
                const int b1offset = eri_cabs_d->offset_a(i1);
                const int b1size = eri_cabs_d->basis_a(i1)->nbasis();

                const int size_j = eri_cabs_d->size_j();
                for (int i2 = 0; i2 != size_j; ++i2) { // OBS
                  const int b2offset = eri_cabs_d->offset_j(i2);
                  const int b2size = eri_cabs_d->basis_j(i2)->nbasis();

                  const int size_b = eri_cabs_d->size_b();
                  for (int i3 = 0; i3 != size_b; ++i3) { // OBS
                    const int b3offset = eri_cabs_d->offset_b(i3);
                    const int b3size = eri_cabs_d->basis_b(i3)->nbasis();

                    const double integral_bound = eri_cabs_d->schwarz_ia(((m1      + k) * size_i + i0) * size_a + i1)
                                                * eri_cabs_d->schwarz_jb(((m3 - m2 + k) * size_j + i2) * size_b + i3);
                    const bool skip_schwarz = integral_bound < SCHWARZ_THRESH;
                    if (skip_schwarz) continue;

                    for (int j0 = b0offset, j0q = b0offset * q; j0 != b0offset + b0size; ++j0, j0q += q) { // center unit cell
                      for (int j1 = b1offset                    ; j1 != b1offset + b1size; ++j1          ) {
                        for (int j2 = b2offset                    ; j2 != b2offset + b2size; ++j2          ) {
                          for (int j3 = b3offset, j3n = b3offset * n; j3 != b3offset + b3size; ++j3, j3n += n, ++cdata) {
                            const double integral2 = *cdata + *cdata;
                            data[m1________k____qb + j0q + j1] += den[m2___m3___k____b + j3n + j2] * integral2;
                          }
                        }
                      }
                    }

                  }
                }
              }
            }
          }

        }
      }
    }
    delete[] diskdata_cs;
  }
  /// cooo loop ends.

  /// coco loop starts here.
  {
    shared_ptr<PCompCABSFile<ERIBatch> >
      eri_cabs_t(new PCompCABSFile<ERIBatch>(geom_, gamma, true, false, true, false, false, "ERI CABS(ia)"));
    eri_cabs_t->store_integrals();
    eri_cabs_t->reopen_with_inout();
    const size_t allocsize_cs = eri_cabs_t->max_num_int();
    double* diskdata_cs = new double[allocsize_cs];

    long file_position = 0l;
    size_t mcnt = 0lu;
    for (int m1 = -s; m1 <= s; ++m1) {
      for (int m2 = -l; m2 <= l; ++m2) { // *NOT* using bra-ket symmetry!!!
        for (int m3 = m2 - s; m3 <= m2 + s; ++m3, ++mcnt) {

          const int k = geom_->K();
          const size_t b = density->blocksize();
          const size_t qb = coulomb_real_space->blocksize();

          const int n = geom_->nbasis();
          const int q = union_geom_->nbasis();

          // for density.
          const int m2___m3___k____b = (m2 - m3 + k) * b;
          const int _____m1___k____b = (   - m1 + k) * b;

          {
            const int m1________k____qb = (m1      + k) * qb;
            const int m3___m2___k____qb = (m3 - m2 + k) * qb;

            eri_cabs_t->get_block(file_position, eri_cabs_t->num_int_each(mcnt), diskdata_cs);
            file_position += eri_cabs_t->num_int_each(mcnt);
            const double* cdata = diskdata_cs;

            const int size_i = eri_cabs_t->size_i();
            for (int i0 = 0; i0 != size_i; ++i0) { // CABS!!!!
              const int b0offset = eri_cabs_t->offset_i(i0) + geom_->nbasis();
              const int b0size = eri_cabs_t->basis_i(i0)->nbasis();

              const int size_a = eri_cabs_t->size_a();
              for (int i1 = 0; i1 != size_a; ++i1) { // CABS!!!
                const int b1offset = eri_cabs_t->offset_a(i1) + geom_->nbasis();
                const int b1size = eri_cabs_t->basis_a(i1)->nbasis();

                const int size_j = eri_cabs_t->size_j();
                for (int i2 = 0; i2 != size_j; ++i2) { // OBS
                  const int b2offset = eri_cabs_t->offset_j(i2);
                  const int b2size = eri_cabs_t->basis_j(i2)->nbasis();

                  const int size_b = eri_cabs_t->size_b();
                  for (int i3 = 0; i3 != size_b; ++i3) { // OBS
                    const int b3offset = eri_cabs_t->offset_b(i3);
                    const int b3size = eri_cabs_t->basis_b(i3)->nbasis();

                    const double integral_bound = eri_cabs_t->schwarz_ia(((m1      + k) * size_i + i0) * size_a + i1)
                                                * eri_cabs_t->schwarz_jb(((m3 - m2 + k) * size_j + i2) * size_b + i3);
                    const bool skip_schwarz = integral_bound < SCHWARZ_THRESH;
                    if (skip_schwarz) continue;

                    for (int j0 = b0offset, j0q = b0offset * q; j0 != b0offset + b0size; ++j0, j0q += q) { // center unit cell
                      for (int j1 = b1offset                    ; j1 != b1offset + b1size; ++j1          ) {
                        for (int j2 = b2offset                    ; j2 != b2offset + b2size; ++j2          ) {
                          for (int j3 = b3offset, j3n = b3offset * n; j3 != b3offset + b3size; ++j3, j3n += n, ++cdata) {
                            const double integral2 = *cdata + *cdata;
                            data[m1________k____qb + j0q + j1] += den[m2___m3___k____b + j3n + j2] * integral2;
                          }
                        }
                      }
                    }

                  }
                }
              }
            }
          }

        }
      }
    }
    delete[] diskdata_cs;
  }

  return coulomb_real_space;
}


RefMatrix PMP2::coulomb_runtime_OBS() const {

  RefMatrix density(new PMatrix1e(coeff_->form_density_rhf()));
  const complex<double>* den = density->data()->front();

  RefMatrix coulomb_real_space(new PMatrix1e(geom_));
  complex<double>* data = coulomb_real_space->data()->front();

  size_t allocsize = eri_obs_->max_num_int();
  double* diskdata = new double[allocsize];

  const int s = geom_->S();
  const int l = geom_->L();

  long file_position = 0l;
  size_t mcnt = 0lu;
  for (int m1 = -s; m1 <= s; ++m1) {
    for (int m2 = 0; m2 <= l; ++m2) { // use bra-ket symmetry!!!
      for (int m3 = m2 - s; m3 <= m2 + s; ++m3, ++mcnt) {

        const int k = geom_->K();
        const size_t b = coulomb_real_space->blocksize();
        assert(coulomb_real_space->blocksize() == density->blocksize());
        const int n = geom_->nbasis();

        const int m1________k____b = (m1      + k) * b;
        const int m1___m2___k____b = (m1 - m2 + k) * b;
        const int m2___m1___k____b = (m2 - m1 + k) * b;
        const int m2___m3___k____b = (m2 - m3 + k) * b;
        const int m3___m2___k____b = (m3 - m2 + k) * b;
        const int m3________k____b = (m3      + k) * b;
        const int _____m1___k____b = (   - m1 + k) * b;
        const int _____m3___k____b = (   - m3 + k) * b;

        {
          eri_obs_->get_block(file_position, eri_obs_->num_int_each(mcnt), diskdata);
          file_position += eri_obs_->num_int_each(mcnt);
          const double* cdata = diskdata;

          const int size = eri_obs_->basissize(); // number of shells
          for (int i0 = 0; i0 != size; ++i0) {
            const int b0offset = eri_obs_->offset(i0);
            const int b0size = eri_obs_->nbasis(i0);

            for (int i1 = 0; i1 != size; ++i1) {
              const int b1offset = eri_obs_->offset(i1);
              const int b1size = eri_obs_->nbasis(i1);

              for (int i2 = 0; i2 != size; ++i2) {
                const int b2offset = eri_obs_->offset(i2);
                const int b2size = eri_obs_->nbasis(i2);

                for (int i3 = 0; i3 != size; ++i3) {
                  const int b3offset = eri_obs_->offset(i3);
                  const int b3size = eri_obs_->nbasis(i3);

                  const double integral_bound = eri_obs_->schwarz(((m1      + k) * size + i0) * size + i1)
                                              * eri_obs_->schwarz(((m3 - m2 + k) * size + i2) * size + i3);
                  const bool skip_schwarz = integral_bound < SCHWARZ_THRESH;
                  if (skip_schwarz) continue;

                  if (m2 != 0) {
                    for (int j0 = b0offset, j0n = b0offset * n; j0 != b0offset + b0size; ++j0, j0n += n) { // center unit cell
                      for (int j1 = b1offset, j1n = b1offset * n; j1 != b1offset + b1size; ++j1, j1n += n) {
                        for (int j2 = b2offset, j2n = b2offset * n; j2 != b2offset + b2size; ++j2, j2n += n) {
                          for (int j3 = b3offset, j3n = b3offset * n; j3 != b3offset + b3size; ++j3, j3n += n, ++cdata) {
                            const double integral2 = *cdata + *cdata;
                            data[m1________k____b + j0n + j1] += den[m2___m3___k____b + j3n + j2] * integral2;
                            data[m3___m2___k____b + j2n + j3] += den[_____m1___k____b + j1n + j0] * integral2;
                          }
                        }
                      }
                    }
                  } else {
                    for (int j0 = b0offset, j0n = b0offset * n; j0 != b0offset + b0size; ++j0, j0n += n) { // center unit cell
                      for (int j1 = b1offset                    ; j1 != b1offset + b1size; ++j1          ) {
                        for (int j2 = b2offset                    ; j2 != b2offset + b2size; ++j2          ) {
                          for (int j3 = b3offset, j3n = b3offset * n; j3 != b3offset + b3size; ++j3, j3n += n, ++cdata) {
                            const double integral2 = *cdata + *cdata;
                            data[m1________k____b + j0n + j1] += den[m2___m3___k____b + j3n + j2] * integral2;
                          }
                        }
                      }
                    }
                  }

                }
              }
            }
          }
        }

      }
    }
  }

  delete[] diskdata;
  RefMatrix out(new PMatrix1e(coulomb_real_space->ft()));
  return out;
}



