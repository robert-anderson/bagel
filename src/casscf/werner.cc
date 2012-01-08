//
// Author : Toru Shiozaki
// Date   : Jan 2012
//


#include <src/casscf/werner.h>
#include <src/casscf/jvec.h>
#include <src/scf/matrix1e.h>
#include <src/casscf/rotfile.h>
#include <src/util/aughess.h>
#include <src/util/linear.h>

using namespace std;

static const double cps = static_cast<double>(CLOCKS_PER_SEC);

void WernerKnowles::compute() {
  const string indent = "  ";

  cout << indent << "=== CASSCF iteration (" + geom_->basisfile() + ") ===" << endl << endl;

  // initializing Hcore matrix (redundant copy, but I can live with it).
  {
    shared_ptr<Hcore> hc(new Hcore(geom_));
    shared_ptr<Fock<1> > fc(new Fock<1>(geom_, hc));
    hcore_ = fc;
  }

  // All the equation numbers refer to those in Werner and Knowles, J. Chem. Phys. 82, 5053 (1985).
  // macro iteration
  mute_stdcout();
  for (int iter = 0; iter != max_iter_; ++iter) {

    int start = ::clock();

    // performs FCI, which also computes one-index transformed integrals 
    fci_->compute();
    fci_->compute_rdm12();
    // get energy
    vector<double> energy = fci_->energy();

#if 1
    // here make a natural orbitals and update the coefficients
    // this effectively updates 1,2RDM and integrals
    const pair<vector<double>, vector<double> > natorb = fci_->natorb_convert();
    // new coefficients
    shared_ptr<Coeff> new_coeff = update_coeff(ref_->coeff(), natorb.first);
    ref_->set_coeff(new_coeff);
    // occupation number of the natural orbitals
    occup_ = natorb.second;
    if (std::abs(occup_.front()-2.0) < 1.0e-16 || std::abs(occup_.back()) < 1.0e-16)
      throw runtime_error("CASSCF does not work so far if occupied orbitals are strictly doubly occupied or empty.");
#else
    occup_ = fci_->rdm1_av()->diag();
#endif


    shared_ptr<Jvec> jvec(new Jvec(fci_, ref_->coeff(), nclosed_, nact_, nvirt_));

    // initially U is unit
    shared_ptr<Matrix1e> U(new Matrix1e(geom_));
    U->unit();

    // denominator
    shared_ptr<Matrix1e> denom;
    {
      shared_ptr<Matrix1e> f;
      shared_ptr<QFile>    fact, factp, gaa;
      shared_ptr<RotFile> denom_;
      one_body_operators(f, fact, factp, gaa, denom_, false);
      denom = denom_->unpack(geom_, 1.0e100);
    }

    // unit matrix is also prepared for convenience
    shared_ptr<Matrix1e> one(new Matrix1e(*U));

    int miter;
    double error;
    for (miter = 0; miter != max_micro_iter_; ++miter) {
      shared_ptr<Matrix1e> T(new Matrix1e(*U - *one));

      shared_ptr<Matrix1e> hcore_mo_(new Matrix1e(*ref_->coeff() % *hcore_ * *ref_->coeff()));

      // compute initial B (Eq. 19)
      shared_ptr<Matrix1e> bvec = compute_bvec(hcore_mo_, fci_, jvec, U, T, ref_->coeff());

      // compute gradient
      shared_ptr<Matrix1e> grad(new Matrix1e(*U%*bvec-*bvec%*U));
      grad->purify_redrotation(nclosed_,nact_,nvirt_);
      const double error_micro = grad->ddot(*grad)/grad->size();
      if (miter == 0) error = error_micro;
      cout << "   -- iter " << setw(4) << miter << "  residual norm : " << setw(20) << setprecision(10) << error_micro << endl;

      if (error_micro < thresh_micro_) break; 

      // initializing a Davidson manager
#if 0
      AugHess<Matrix1e> solver(max_mmicro_iter_+2, grad); 
#else
      Linear<Matrix1e> solver(max_mmicro_iter_+2, grad); 
#endif

      // update C = 1/2(A+A^dagger) = 1/2(U^dagger B + B^dagger U)
      shared_ptr<Matrix1e> C(new Matrix1e((*U % *bvec + *bvec % *U)*0.5));

      // initial dR value.
      shared_ptr<Matrix1e> dR(new Matrix1e(*grad));
      for (int i = 0; i != dR->size(); ++i) dR->data(i) /=  max(std::abs(denom->data(i)),0.1);

      // solving Eq. 30 of Werner and Knowles
      // fixed U, solve for dR.
      for (int mmiter = 0; mmiter != max_mmicro_iter_; ++mmiter) {

        const int mstart = ::clock();
        // compute U dR
        shared_ptr<Matrix1e> UdR(new Matrix1e(*U**dR));

        // update B
        shared_ptr<Matrix1e> new_bvec = compute_bvec(hcore_mo_, fci_, jvec, UdR, UdR, ref_->coeff());

        // compute  C dR + dR C
        shared_ptr<Matrix1e> dRA(new Matrix1e(*C**dR+*dR**C));

        // compute U^dagger B - B^dagger U
        // compute Eq.29
        shared_ptr<Matrix1e> sigma(new Matrix1e(*U%*new_bvec-*new_bvec%*U-*dRA));
        sigma->purify_redrotation(nclosed_,nact_,nvirt_);

        shared_ptr<Matrix1e> residual = solver.compute_residual(dR, sigma);

        const double error_mmicro = residual->ddot(*residual) / residual->size();
        const double mic_energy = 0.0; // TODO

        const int mend = ::clock();
#if 1
        if (mmiter == 0) cout << endl << "     == micro iteration == " << endl;
        cout << setw(10) << mmiter << "   " << setw(20) << setprecision(12)
             << setw(20) << setprecision(14) << error_mmicro << " " << setprecision(2) << (mend - mstart)/cps << endl;
#endif

        if (error_mmicro < thresh_mmicro_) { cout << endl; break; }
        if (mmiter+1 == max_mmicro_iter_) { cout << endl; break; } 

        // update dR;
        for (int i = 0; i != residual->size(); ++i) residual->data(i) /=  max(std::abs(denom->data(i)),0.1);
        solver.orthog(residual);
        dR = residual;
        dR->purify_redrotation(nclosed_,nact_,nvirt_);

      }
      dR = solver.civec();
      shared_ptr<Matrix1e> dU = dR->exp();
      dU->purify_unitary();
      *U *= *dU;

    }
    shared_ptr<Coeff> newcc(new Coeff(*ref_->coeff() * *U));
    ref_->set_coeff(newcc);

    int end = ::clock();
    if (nstate_ != 1 && iter) cout << endl;
    for (int i = 0; i != nstate_; ++i) {
      resume_stdcout();
      cout << indent << setw(5) << iter << setw(3) << i << setw(4) << miter
                                        << setw(15) << fixed << setprecision(8) << energy[i] << "   "
                                        << setw(10) << scientific << setprecision(2) << (i==0 ? error : 0.0) << fixed << setw(10) << setprecision(2)
                                        << (end - start)/cps << endl;
      mute_stdcout();
    }
    if (error < thresh_) break;

  }
  resume_stdcout();

}



// compute B according to Eq. (19).
// B = 2[h_rs U_sj D_ji + (rs|D)Usj <D|ji> + 2(rk|D)(D|ls)T_sj D_jl,ik]
shared_ptr<Matrix1e> WernerKnowles::compute_bvec(shared_ptr<Matrix1e> hcore, shared_ptr<FCI> fci, shared_ptr<Jvec> jvec,
                                                 shared_ptr<Matrix1e> u, shared_ptr<Matrix1e> t, shared_ptr<Coeff> cc) {
  shared_ptr<DensityFit> df = geom_->df();
  const int naux = df->naux();
  const int nbas = df->nbasis();
  shared_ptr<Matrix1e> out(new Matrix1e(geom_));
  {
    // first term
    Matrix1e all1(geom_);
    for (int i = 0; i != nclosed_; ++i) all1.element(i,i) = 2.0;
    for (int i = 0; i != nact_; ++i) dcopy_(nact_, fci->rdm1_av()->data()+nact_*i, 1, all1.element_ptr(nclosed_, i+nclosed_), 1); 
    shared_ptr<Matrix1e> buf(new Matrix1e(geom_));
    dgemm_("N", "N", nbasis_, nocc_, nbasis_, 1.0, hcore->data(), nbasis_, u->data(), nbasis_, 0.0, buf->data(), nbasis_);  
    dgemm_("N", "N", nbasis_, nocc_, nocc_, 2.0, buf->data(), nbasis_, all1.data(), nbasis_, 0.0, out->data(), nbasis_);
  }

  unique_ptr<double[]> tmp(new double[nbas*nbas]);
  // second term
  {
    Matrix1e Umat(*cc * *u);
    shared_ptr<DF_Half> half = df->compute_half_transform(Umat.data(), nocc_);
    half->form_2index(tmp, jvec->jvec()->apply_J(), 2.0, 0.0);
  }

  // thrid term
  if (t->norm() > 1.0e-15) {
    Matrix1e Tmat(*cc * *t);
    shared_ptr<DF_Full> full = jvec->half()->compute_second_transform(Tmat.data(), nocc_)->apply_2rdm(jvec->rdm2_all());
    jvec->half()->form_2index(tmp, full, 4.0, 1.0);
  }

  dgemm_("T", "N", nbasis_, nocc_, nbas, 1.0, cc->data(), nbas, tmp.get(), nbas, 1.0, out->data(), nbasis_);

  return out;
}

