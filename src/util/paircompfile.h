//
// Author : Toru Shiozaki
// Date   : August 2009
//

// compressed file of double 

#ifndef __src_util_paircompfile_h
#define __src_util_paircompfile_h

#include <src/util/pcompfile.h>

template<class T>
class PairCompFile {
  protected:
    const double param_;
    const std::string jobname_;
    std::pair<boost::shared_ptr<PCompFile<T> >,
              boost::shared_ptr<PCompFile<T> > > files_;

    void init_schwarz();
    void calculate_num_int_each();
    void eval_new_block(double*, double*, int, int, int);

  public:
    PairCompFile(boost::shared_ptr<PGeometry>, const double, const std::string); 
    ~PairCompFile(); 
    void store_integrals();
    void reopen_with_inout();

    boost::shared_ptr<PCompFile<T> > first() { return files_.first; };
    boost::shared_ptr<PCompFile<T> > second() { return files_.second; };

};


template<class T>
PairCompFile<T>::PairCompFile(boost::shared_ptr<PGeometry> gm, const double pr, const std::string jobname)
 : param_(pr), jobname_(jobname) {
  // late_init = "true"
  boost::shared_ptr<PCompFile<T> > first(new PCompFile<T>(gm, true)); 
  boost::shared_ptr<PCompFile<T> > second(new PCompFile<T>(gm, true)); 
  files_ = std::make_pair(first, second);

  // set schwarz for both pcompfiles.
  init_schwarz();

  // set num_int_each in both pcompfiles
  calculate_num_int_each();

};


template<class T>
PairCompFile<T>::~PairCompFile() {

};


template<class T>
void PairCompFile<T>::init_schwarz() {
  typedef boost::shared_ptr<Shell> RefShell;
  typedef boost::shared_ptr<Atom> RefAtom;

  const int size = files_.first->basissize(); // the number of shells per unit cell
  const int K = files_.first->K(); 
  std::vector<double> schwarz1(size * size * (2 * K + 1));
  std::vector<double> schwarz2(size * size * (2 * K + 1));

  for (int m = - K; m <= K; ++m) { 
    const double disp[3] = {0.0, 0.0, m * files_.first->A()};
    for (int i0 = 0; i0 != size; ++i0) { // center unit cell
      const RefShell b0 = files_.first->basis(i0);
      for (int i1 = 0; i1 != size; ++i1) {
        const RefShell b1 = files_.first->basis(i1)->move_atom(disp);

        std::vector<RefShell> input;
        input.push_back(b0);
        input.push_back(b1);
        input.push_back(b0);
        input.push_back(b1);
        T batch(input, 1.0e100, param_, true);
        batch.compute();
        const double* data1 = batch.data();
        const double* data2 = batch.data2();
        const int datasize = batch.data_size();
        double cmax1 = 0.0;
        double cmax2 = 0.0;
        for (int xi = 0; xi != datasize; ++xi, ++data1, ++data2) {
          const double absed1 = (*data1) > 0.0 ? *data1 : -*data1;
          const double absed2 = (*data2) > 0.0 ? *data2 : -*data2;
          if (absed1 > cmax1) cmax1 = absed1;
          if (absed2 > cmax2) cmax2 = absed2;
        }
        schwarz1[(m + K) * size * size + i0 * size + i1] = cmax1;
        schwarz2[(m + K) * size * size + i0 * size + i1] = cmax2;
      }
    }
  }

  files_.first->set_schwarz(schwarz1);
  files_.second->set_schwarz(schwarz2);

#if 0
  std::cout << "debug in util/paircompfile.h" << std::endl;
  std::cout << std::scientific << files_.first->schwarz(0) << std::endl;
#endif
   
};


template<class T>
void PairCompFile<T>::calculate_num_int_each() {

  typedef boost::shared_ptr<Shell> RefShell;

  const int K = files_.first->K();
  const int S = files_.first->S();
  const int L = files_.first->L();
  std::vector<size_t> num_int_each1((S + S + 1) * (S + S + 1) * (L + 1));
  std::vector<size_t> num_int_each2((S + S + 1) * (S + S + 1) * (L + 1));
  std::vector<size_t>::iterator niter1 = num_int_each1.begin();
  std::vector<size_t>::iterator niter2 = num_int_each2.begin();

  size_t data_written1 = 0ul;
  size_t data_written2 = 0ul;

  const int size = files_.first->basissize(); // number of shells

  boost::shared_ptr<PCompFile<T> > first = files_.first;
  boost::shared_ptr<PCompFile<T> > second = files_.second;

  for (int m1 = - S; m1 <= S; ++m1) {
    const double A = first->A();
    const double m1disp[3] = {0.0, 0.0, m1 * A}; 
    for (int m2 = 0; m2 <= L; ++m2) { // use bra-ket symmetry!!!
      const double m2disp[3] = {0.0, 0.0, m2 * A}; 
      for (int m3 = m2 - S; m3 <= m2 + S; ++m3, ++niter1, ++niter2) {
        const double m3disp[3] = {0.0, 0.0, m3 * A}; 
        size_t thisblock1 = 0ul; 
        size_t thisblock2 = 0ul; 
        for (int i0 = 0; i0 != size; ++i0) {
          const int b0offset = first->offset(i0); 
          const int b0size = first->basis(i0)->nbasis();

          for (int i1 = 0; i1 != size; ++i1) {
            const int b1offset = first->offset(i1); 
            const int b1size = first->basis(i1)->nbasis();

            for (int i2 = 0; i2 != size; ++i2) {
              const int b2offset = first->offset(i2); 
              const int b2size = first->basis(i2)->nbasis();

              for (int i3 = 0; i3 != size; ++i3) {
                const int b3offset = first->offset(i3); 
                const int b3size = first->basis(i3)->nbasis();

                {
                  const double integral_bound = first->schwarz((m1 + K) * size * size + i0 * size + i1)
                                              * first->schwarz((m3 - m2 + K) * size * size + i2 * size + i3);
                  const bool skip_schwarz = integral_bound < SCHWARZ_THRESH;
                  if (!skip_schwarz) { 
                    data_written1 += b0size * b1size * b2size * b3size;
                    thisblock1 += b0size * b1size * b2size * b3size; 
                  }
                }
                {
                  const double integral_bound = second->schwarz((m1 + K) * size * size + i0 * size + i1)
                                              * second->schwarz((m3 - m2 + K) * size * size + i2 * size + i3);
                  const bool skip_schwarz = integral_bound < SCHWARZ_THRESH;
                  if (!skip_schwarz) { 
                    data_written2 += b0size * b1size * b2size * b3size;
                    thisblock2 += b0size * b1size * b2size * b3size; 
                  }
                }

              }
            }
          }
        }
        *niter1 = thisblock1;
        *niter2 = thisblock2;
      }
    }
  }

#if 0
  const size_t max_num_int1 = *std::max_element(num_int_each1.begin(), num_int_each1.end());
  const size_t max_num_int2 = *std::max_element(num_int_each2.begin(), num_int_each2.end());

  std::cout << "debug in util/paircompfile.h" << std::endl;
  std::cout << max_num_int1 << " " << max_num_int2 << std::endl;
  std::cout << data_written1 << " " << data_written2 << std::endl;
#endif

  first->set_num_int_each(num_int_each1);
  second->set_num_int_each(num_int_each2);

  std::cout << "  Using ";
  const size_t data_written_byte = (data_written1 + data_written2) * sizeof(double);
  if (data_written_byte > 1.0e9) {
    std::cout << std::setprecision(1) << data_written_byte / 1.0e9 << " GB";
  } else if (data_written_byte > 1.0e6) {
    std::cout << std::setprecision(1) << data_written_byte / 1.0e6 << " MB";
  } else {
    std::cout << std::setprecision(1) << data_written_byte / 1.0e3 << " KB";
  }

  std::cout << " hard disk for storing \"" << jobname_ << "\"" << std::endl; 
  std::cout << std::endl;
  assert(std::max(data_written1, data_written2) < 5.0e9); // 40GB
};


template<class T>
void PairCompFile<T>::eval_new_block(double* out1, double* out2, int m1, int m2, int m3) {

  typedef boost::shared_ptr<Shell> RefShell;

  boost::shared_ptr<PCompFile<T> > first = files_.first;
  boost::shared_ptr<PCompFile<T> > second = files_.second;

  const int K = first->K();
  const double A = first->A();
  const double m1disp[3] = {0.0, 0.0, m1 * A}; 
  const double m2disp[3] = {0.0, 0.0, m2 * A}; 
  const double m3disp[3] = {0.0, 0.0, m3 * A}; 

  const int size = first->basissize(); // number of shells
  size_t data_acc1 = 0lu;
  size_t data_acc2 = 0lu;
  for (int i0 = 0; i0 != size; ++i0) {
    const RefShell b0 = first->basis(i0); // b0 is the center cell
    const int b0offset_ = first->offset(i0); 
    const int b0size = b0->nbasis();

    for (int i1 = 0; i1 != size; ++i1) {
      const RefShell b1 = first->basis(i1)->move_atom(m1disp); 
      const int b1offset_ = first->offset(i1);
      const int b1size = b1->nbasis();

      for (int i2 = 0; i2 != size; ++i2) {
        const RefShell b2 = first->basis(i2)->move_atom(m2disp);
        const int b2offset_ = first->offset(i2);
        const int b2size = b2->nbasis();

        for (int i3 = 0; i3 != size; ++i3) {
          const RefShell b3 = first->basis(i3)->move_atom(m3disp);
          const int b3offset_ = first->offset(i3);
          const int b3size = b3->nbasis();
        
          const double integral_bound1 = first->schwarz((m1      + K) * size * size + i0 * size + i1)
                                       * first->schwarz((m3 - m2 + K) * size * size + i2 * size + i3);
          const double integral_bound2 = second->schwarz((m1      + K) * size * size + i0 * size + i1)
                                       * second->schwarz((m3 - m2 + K) * size * size + i2 * size + i3);
          const bool skip_schwarz1 = integral_bound1 < SCHWARZ_THRESH;
          const bool skip_schwarz2 = integral_bound2 < SCHWARZ_THRESH;

          const bool skip_schwarz = std::max(integral_bound1, integral_bound2) < SCHWARZ_THRESH;
          assert(integral_bound1 >= 0.0 && integral_bound2 >= 0.0 && SCHWARZ_THRESH >= 0);
          if (skip_schwarz) continue;

          std::vector<RefShell> input;
          input.push_back(b3);
          input.push_back(b2);
          input.push_back(b1);
          input.push_back(b0);

          if (!skip_schwarz1 && skip_schwarz2) {
            T batch(input, 1.0, param_, false);
            batch.compute();
            const double* bdata1 = batch.data();
            size_t current_size = b0size * b1size * b2size * b3size;
            ::memcpy(out1 + data_acc1, bdata1, current_size * sizeof(double));
            data_acc1 += current_size; 
          } else {
            T batch(input, 1.0, param_, true);
            batch.compute();
            const double* bdata1 = batch.data();
            const double* bdata2 = batch.data2();
            size_t current_size = b0size * b1size * b2size * b3size;
            ::memcpy(out1 + data_acc1, bdata1, current_size * sizeof(double));
            ::memcpy(out2 + data_acc2, bdata2, current_size * sizeof(double));
            data_acc1 += current_size; 
            data_acc2 += current_size; 
          }

        }
      }
    }
  }
};


template<class T>
void PairCompFile<T>::store_integrals() {
  boost::shared_ptr<PCompFile<T> > first = files_.first;
  boost::shared_ptr<PCompFile<T> > second = files_.second;
  double* dcache1 = new double[first->max_num_int()];
  double* dcache2 = new double[second->max_num_int()];
  const int S = first->S();
  const int L = first->L();
  size_t cnt = 0;
  for (int m1 = -S; m1 <= S; ++m1) {
    for (int m2 = 0; m2 <= L; ++m2) { // use bra-ket symmetry!!!
      for (int m3 = m2 - S; m3 <= m2 + S; ++m3, ++cnt) {
        eval_new_block(dcache1, dcache2, m1, m2, m3);
        first->append(first->num_int_each(cnt), dcache1);
        second->append(second->num_int_each(cnt), dcache2);
      }
    }
  } 
  delete[] dcache1;
  delete[] dcache2;
};


template<class T>
void PairCompFile<T>::reopen_with_inout() {
  files_.first->reopen_with_inout();
  files_.second->reopen_with_inout();
};

#endif

