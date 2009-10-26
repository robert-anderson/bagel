//
// Author : Toru Shiozaki
// Date   : September 2009
//

#pragma once
#ifndef __src_util_pmofile_h
#define __src_util_pmofile_h

#include <src/pscf/pgeometry.h>
#include <src/util/pfile.h>
#include <src/pscf/f77.h>

template<class T>
class PMOFile : public PFile<T> {
  protected:
    const int istart_;
    const int jstart_;
    const int astart_;
    const int bstart_;
    const int ifence_;
    const int jfence_;
    const int afence_;
    const int bfence_;

    const boost::shared_ptr<PGeometry> geom_;

  public:
    PMOFile(const boost::shared_ptr<PGeometry>,
        const long, const int, const int, const int,
        const int, const int,
        const int, const int,
        const int, const int,
        const bool late_init = false);
    ~PMOFile(); 

    void sort_inside_blocks();
    PMOFile<T> operator+(const PMOFile<T>&) const;
    PMOFile<T> operator-(const PMOFile<T>&) const;
    PMOFile<T> operator*(const std::complex<double>&) const;
    boost::shared_ptr<PMOFile<T> > flip() const;

    boost::shared_ptr<PMOFile<T> > contract(boost::shared_ptr<PMOFile<T> >, std::string,
                                            const int i = 0, const int j = 0, const int a = 0, const int b = 0,
                                            const int m = 0, const int n = 0);

    boost::shared_ptr<PMatrix1e> contract12(const boost::shared_ptr<PMatrix1e>) const;

    void print() const;
    void rprint() const;
    T get_energy_one_amp() const;

};


template<class T>
PMOFile<T>::PMOFile(const boost::shared_ptr<PGeometry> gm,
                    const long fsize, const int k,
                    const int istrt, const int ifen,
                    const int jstrt, const int jfen,
                    const int astrt, const int afen,
                    const int bstrt, const int bfen,
                    const bool late_init)
 : PFile<T>(fsize, k, late_init), geom_(gm),
   istart_(istrt), ifence_(ifen), jstart_(jstrt), jfence_(jfen),
   astart_(astrt), afence_(afen), bstart_(bstrt), bfence_(bfen) {


};


template<class T>
PMOFile<T>::~PMOFile() {
};


// TODO conflicts in notation
// sort to chemist notations... but outermost block loops remain unchanged...
// that is unfavorable and needs some fix in PCompFile::mo_transform...
template<class T>
void PMOFile<T>::sort_inside_blocks() {
  const int k = this->K_;
  const int KKK8 = std::max(k * k * k * 8, 1);
  assert(this->filesize_ % KKK8 == 0);

  const size_t blocksize = this->filesize_ / KKK8;
  size_t current = 0lu; 

  T* buffer1 = new T[blocksize];
  T* buffer2 = new T[blocksize];

  const int isize = ifence_ - istart_;
  const int jsize = jfence_ - jstart_;
  const int asize = afence_ - astart_;
  const int bsize = bfence_ - bstart_;
  const size_t absize = asize * bsize;

  for (int iblock = 0; iblock != KKK8; ++iblock, current += blocksize) { 
    this->get_block(current, blocksize, buffer1); 
    #pragma omp parallel for
    for (int i = 0; i < isize; ++i) {
      const T* cbuf = buffer1 + bsize * asize * jsize * i;
      for (int a = 0; a != asize; ++a) {
        T* cbuf2 = buffer2 + bsize * (a + asize * (0 + jsize * i));
        for (int j = 0; j != jsize; ++j, cbuf += bsize, cbuf2 += absize) {
          ::memcpy(cbuf2, cbuf, bsize * sizeof(T));
        }
      }
    }
    this->put_block(current, blocksize, buffer2);
  }

  delete[] buffer1;
  delete[] buffer2;
};


template<class T>
boost::shared_ptr<PMOFile<T> > PMOFile<T>::flip() const {
  const int k = this->K_;
  const int KKK8 = std::max(k * k * k * 8, 1);
  assert(this->filesize_ % KKK8 == 0);

  const size_t blocksize = this->filesize_ / KKK8;
  size_t current = 0lu;

  T* buffer1 = new T[blocksize];
  T* buffer2 = new T[blocksize];

  const int isize = ifence_ - istart_;
  const int jsize = jfence_ - jstart_;
  const int asize = afence_ - astart_;
  const int bsize = bfence_ - bstart_;

  boost::shared_ptr<PMOFile<T> > out(new PMOFile<T>(geom_, this->filesize_, k,
                                                    jstart_, jfence_, istart_, ifence_,
                                                    bstart_, bfence_, astart_, afence_, false));

  for (int iblock = 0; iblock != KKK8; ++iblock, current += blocksize) {
    this->get_block(current, blocksize, buffer1);
    T* cbuf1 = buffer1;
    for (int i = 0; i < isize; ++i) {
      for (int j = 0; j < jsize; ++j) {
        for (int a = 0; a < asize; ++a) {
          for (int b = 0; b < bsize; ++b, ++cbuf1) {
            T* cbuf2 = buffer2 + a + asize * (b + bsize * (i + isize * j));
            assert(cbuf1 == buffer1 + b + bsize * (a + asize * (j + jsize * i)));
            *cbuf2 = *cbuf1;
          }
        }
      }
    }
    out->put_block(current, blocksize, buffer2);
  }

  delete[] buffer1;
  delete[] buffer2;
  return out;
};


template<class T>
boost::shared_ptr<PMOFile<T> > PMOFile<T>::contract(boost::shared_ptr<PMOFile<T> > other, std::string jobname,
                                                    const int ioffset, const int joffset,
                                                    const int aoffset, const int boffset,
                                                    const int moffset, const int noffset) {

  std::cout << "  Entering " << jobname << " contraction..." << std::endl;
  const int k = this->K_;
  const int KKK8 = std::max(k * k * k * 8, 1);
  assert(this->filesize_ % KKK8 == 0);
  const size_t blocksize1 = this->filesize_ / KKK8;
  const size_t blocksize2 = other->filesize_ / KKK8;

  T* buffer1 = new T[blocksize1];
  T* buffer2 = new T[blocksize2];

  const int isize = ifence_ - istart_;
  const int jsize = jfence_ - jstart_;
  const int asize = afence_ - astart_;
  const int bsize = bfence_ - bstart_;
  const int mfence = other->ifence_;
  const int mstart = other->istart_;
  const int msize = mfence - mstart;
  const int nfence = other->jfence_;
  const int nstart = other->jstart_;
  const int nsize = nfence - nstart;
  const int ijsize = isize * jsize;
  const int absize = asize * bsize;
  const int mnsize = msize * nsize;

  assert(blocksize1 == ijsize * absize && blocksize2 == mnsize * absize);

  T* target = new T[ijsize * mnsize];

  const size_t out_blocksize = ijsize * mnsize;
  const size_t out_filesize = KKK8 * out_blocksize; 
  boost::shared_ptr<PMOFile<T> > out(new PMOFile<T>(geom_, out_filesize, k, istart_, ifence_, jstart_, jfence_,
                                                    mstart, mfence, nstart, nfence, false));

  const int k2 = std::max(k + k, 1);

  size_t current = 0lu; 
  for (int kj = -k; kj < std::max(k, 1); ++kj) { 
    for (int ki = -k; ki < std::max(k, 1); ++ki) {
      for (int kn = -k; kn < std::max(k, 1); ++kn) { 
        for (int km = -k; km < std::max(k, 1); ++km) {
          if ((km + kn - ki - kj) % k2 != 0) continue; 

          // block index is in Physicists' notation!!!
          const T zero = 0.0;
          fill(target, target + ijsize * mnsize, zero);

          for (int ka = -k; ka < std::max(k, 1); ++ka) {
            for (int kb = -k; kb < std::max(k, 1); ++kb) {
              if ((ka + kb - ki - kj) % k2 != 0) continue; 

              this->get_block(blocksize1 * (kb + k + k2 * (ki + k + k2 * (ka + k))), blocksize1, buffer1);
              other->get_block(blocksize2 * (kb + k + k2 * (km + k + k2 * (ka + k))), blocksize2, buffer2);

              const T one = 1.0;
              const T prefac = one / static_cast<T>(k2);
              zgemm_("C", "N", &ijsize, &mnsize, &absize, &prefac, buffer1, &absize, buffer2, &absize, &one, target, &ijsize);
            }
          }

          const size_t kinj = kj + k + k2 * (km + k + k2 * (ki + k));
          out->put_block(kinj * out_blocksize, out_blocksize, target);
        }
      }
    }
  }

  delete[] buffer1;
  delete[] buffer2;

  delete[] target;

  std::cout << "  done" << std::endl << std::endl;

  return out;
};


template<class T>
PMOFile<T> PMOFile<T>::operator-(const PMOFile<T>& other) const {

  const int k = this->K_;
  const size_t fsize = this->filesize_;
  const int kkk8 = std::max(k * k * k * 8, 1);
  const size_t blocksize = fsize / kkk8;
  T* buffer1 = new T[blocksize];
  T* buffer2 = new T[blocksize];
  assert(fsize % kkk8 == 0);

  PMOFile out(geom_, fsize, k, istart_, ifence_, jstart_, jfence_, astart_, afence_, bstart_, bfence_, false);

  size_t current = 0lu;
  for (int iblock = 0; iblock != kkk8; ++iblock, current += blocksize) {
    this->get_block(current, blocksize, buffer1); 
    other.get_block(current, blocksize, buffer2);

    #pragma omp parallel for schedule(dynamic, 100)
    for (int i = 0; i < blocksize; ++i) buffer1[i] -= buffer2[i];

    out.put_block(current, blocksize, buffer1); 
  }

  delete[] buffer1;
  delete[] buffer2;
  return out;
};


template<class T>
PMOFile<T> PMOFile<T>::operator+(const PMOFile<T>& other) const {

  const int k = this->K_;
  const size_t fsize = this->filesize_;
  const int kkk8 = std::max(k * k * k * 8, 1);
  const size_t blocksize = fsize / kkk8;
  T* buffer1 = new T[blocksize];
  T* buffer2 = new T[blocksize];
  assert(fsize % kkk8 == 0);

  PMOFile out(geom_, fsize, k, istart_, ifence_, jstart_, jfence_, astart_, afence_, bstart_, bfence_, false);

  size_t current = 0lu;
  for (int iblock = 0; iblock != kkk8; ++iblock, current += blocksize) {
    this->get_block(current, blocksize, buffer1); 
    other.get_block(current, blocksize, buffer2);

    #pragma omp parallel for schedule(dynamic, 100)
    for (int i = 0; i < blocksize; ++i) buffer1[i] += buffer2[i];

    out.put_block(current, blocksize, buffer1); 
  }

  delete[] buffer1;
  delete[] buffer2;
  return out;
};


template<class T>
PMOFile<T> PMOFile<T>::operator*(const std::complex<double>& a) const {

  const int k = this->K_;
  const size_t fsize = this->filesize_;
  const int kkk8 = std::max(k * k * k * 8, 1);
  const size_t blocksize = fsize / kkk8;
  T* buffer1 = new T[blocksize];
  assert(fsize % kkk8 == 0);

  PMOFile out(geom_, fsize, k, istart_, ifence_, jstart_, jfence_, astart_, afence_, bstart_, bfence_, false);

  size_t current = 0lu;
  for (int iblock = 0; iblock != kkk8; ++iblock, current += blocksize) {
    this->get_block(current, blocksize, buffer1);

    #pragma omp parallel for schedule(dynamic, 100)
    for (int i = 0; i < blocksize; ++i) buffer1[i] *= a;

    out.put_block(current, blocksize, buffer1);
  }

  delete[] buffer1;
  return out;
};


template<class T>
void PMOFile<T>::print() const {

  const int isize = ifence_ - istart_;
  const int jsize = jfence_ - jstart_;
  const int asize = afence_ - astart_;
  const int bsize = bfence_ - bstart_;
  const size_t ijsize = isize * jsize;
  const size_t absize = asize * bsize;
  const int k = this->K_;
  const int kk = std::max(k + k, 1);
  T* buffer = new T[ijsize * absize];

  // I will print out in ne case...
  if (isize * jsize <= 25) {
    int iall = 0;
    for (int ki = -k; ki != std::max(k, 1); ++ki) {
      for (int kj = -k; kj != std::max(k, 1); ++kj) {
        for (int ka = -k; ka != std::max(k, 1); ++ka, ++iall) {
          std::cout << std::endl;
          this->get_block(iall * ijsize * absize, ijsize * absize, buffer);
          const T* cbuf = buffer;
          for (int i = 0; i != ijsize; ++i) {
            for (int j = 0; j != absize; ++j, ++cbuf) {
              std::cout << std::setprecision(4) << std::setw(15) << *cbuf;
            }
            std::cout << std::endl;
          }
          std::cout << std::endl;
        }
      }
    }
  }
  delete[] buffer;
};


template<class T>
void PMOFile<T>::rprint() const {
  const int isize = ifence_ - istart_;  
  const int jsize = jfence_ - jstart_;  
  const size_t ijsize = isize * jsize;
  const int k = this->K_;
  T* buffer = new T[ijsize * ijsize];

  // I will print out in Ne or H2O cases...
  // singlet
  if (isize * jsize <= 25) {
    int iall = 0;
    for (int ki = -k; ki != std::max(k, 1); ++ki) {
      for (int kj = -k; kj != std::max(k, 1); ++kj) {
        for (int ka = -k; ka != std::max(k, 1); ++ka, ++iall) {
          std::cout << "  block " << ki << " : " << kj << " : " << ka << std::endl;
          this->get_block(iall * ijsize * ijsize, ijsize * ijsize, buffer); 
          const T* cbuf = buffer;
          for (int i = 0; i != ijsize; ++i) {
            for (int j = 0; j != ijsize; ++j, ++cbuf) {
              std::cout << std::setprecision(5) << std::setw(9) << (*cbuf).real();
            }
            std::cout << std::endl;
          }
          std::cout << std::endl;
        }
      }
    }
  }
  delete[] buffer;
};


template<class T>
T PMOFile<T>::get_energy_one_amp() const {
  const int isize = ifence_ - istart_;  
  const int jsize = jfence_ - jstart_;  
  const size_t ijsize = isize * jsize;
  assert(isize == jsize && isize == (afence_ - astart_) && jsize == (bfence_ - astart_));
  const int k = this->K_;
  T* buffer = new T[ijsize * ijsize];
  assert(geom_->gamma() > 0.0);
  const double gamma = geom_->gamma();
  const double ciiii = - 0.5 / gamma;
  const double cijij = - 0.375 / gamma;
  const double cijji = - 0.125 / gamma;
  int iblock = 0;
  T en = 0.0;
  for (int kb = -k; kb != std::max(k, 1); ++kb) {
    for (int kj = -k; kj != std::max(k, 1); ++kj) {
      for (int ka = -k; ka != std::max(k, 1); ++ka, ++iblock) {
        if (kb == kj && kb == ka) {
          this->get_block(iblock * ijsize * ijsize, ijsize * ijsize, buffer);
          for (int i1 = 0; i1 != isize; ++i1) { // i
            for (int i2 = 0; i2 != isize; ++i2) { // j
              if (i1 == i2) {
                const int xi = i2 + i1 * isize;
                const int xj = i2 + i1 * isize;
                en += buffer[xi * ijsize + xj] * ciiii;
              } else {
                const int xi = i2 + i1 * isize;
                const int xj1 = i2 + i1 * isize;
                const int xj2 = i1 + i2 * isize;
                en += buffer[xi * ijsize + xj1] * (2 * cijij - cijji);
                en += buffer[xi * ijsize + xj2] * (2 * cijji - cijij);
              }
            }
          }
        } else if (kb == kj && kb != ka) {
          this->get_block(iblock * ijsize * ijsize, ijsize * ijsize, buffer);
          for (int i1 = 0; i1 != isize; ++i1) { // i
            for (int i2 = 0; i2 != isize; ++i2) { // j
              const int xi = i2 + i1 * isize;
              const int xj = i2 + i1 * isize;
              en += buffer[xi * ijsize + xj] * (2 * cijij - cijji);
            }
          }
        } else if (ka == kj && kb != ka) {
          this->get_block(iblock * ijsize * ijsize, ijsize * ijsize, buffer);
          for (int i1 = 0; i1 != isize; ++i1) { // i
            for (int i2 = 0; i2 != isize; ++i2) { // j
              const int xi = i2 + i1 * isize;
              const int xj = i1 + i2 * isize;
              en += buffer[xi * ijsize + xj] * (2 * cijji - cijij);
            }
          }
        }
      }
    }
  }
  delete[] buffer;
  en /= static_cast<T>(std::max(k * k * 4, 1));
  return en;
};


// PMatrix1e assumes complex<double>.
template <class T>
boost::shared_ptr<PMatrix1e> PMOFile<T>::contract12(const boost::shared_ptr<PMatrix1e> density) const {
  typedef boost::shared_ptr<PMatrix1e> RefMatrix;

  // designed for operations like:
  // d^a_i * v^ij_ab -> h^j_b

  const int isize = ifence_ - istart_;
  const int jsize = jfence_ - jstart_;
  const int asize = afence_ - astart_;
  const int bsize = bfence_ - bstart_;
  const size_t ijsize = isize * jsize;
  const size_t absize = asize * bsize;

  // will be removed.
  assert(isize == asize);

  RefMatrix out(new PMatrix1e(geom_, bsize, jsize));

  const int k = this->K_;
  const size_t blocksize = absize*ijsize;
  std::complex<double>* buffer = new std::complex<double>[blocksize];
  assert(this->filesize_/std::max(k*k*k*8, 1) == absize*ijsize);

  for (int kb = -k, iall = 0; kb != std::max(k, 1); ++kb) {
    for (int kj = -k; kj != std::max(k, 1); ++kj) {
      for (int ka = -k; ka != std::max(k, 1); ++ka, ++iall) {
        int ki = ka+kb-kj;
        if (ki < -k) ki += k*2;
        else if (ki >=  k) ki -= k*2;

        int kd = ki-ka;
        if (kd < -k) kd += k*2;
        else if (kd >= k) kd -= k*2;

        int ko = kb-kj;
        if (ko < -k) ko += k*2;
        else if (ko >= k) ko -= k*2;


        const std::complex<double>* dblock = density->bp(kd);
        std::complex<double>* oblock = out->bpw(ko);

        this->get_block(iall*blocksize, blocksize, buffer);
        const std::complex<double>* cbuf = buffer;
        // Assumes that it is already sorted.
        for (int i = 0; i != isize; ++i) {
          for (int j = 0; j != jsize; ++j) {
            for (int a = 0; a != asize; ++a) {
              const std::complex<double> cden = dblock[i + isize*a];
              for (int b = 0; b != bsize; ++b, +cbuf) {
                oblock[b + bsize*j] += *cbuf * cden;
              }
            }
          }
        }
      }
    }
  }

  delete[] buffer;
  return out;
};

#endif
