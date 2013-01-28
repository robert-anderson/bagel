//
// BAGEL - Parallel electron correlation program.
// Filename: test_opt.cc
// Copyright (C) 2012 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and\/or modify
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

#include <src/opt/opt.h>
#include <src/scf/scf.h>
#include <src/wfn/reference.h>

std::vector<double> run_opt(std::string filename) {

  std::string outputname = filename + ".testout";
  std::string inputname = "../../test/" + filename + ".in";
  std::shared_ptr<std::ofstream> ofs(new std::ofstream(outputname, std::ios::trunc));
  std::streambuf* backup_stream = std::cout.rdbuf(ofs->rdbuf());

  // a bit ugly to hardwire an input file, but anyway...
  std::shared_ptr<InputData> idata(new InputData(inputname));
  std::shared_ptr<Geometry> geom(new Geometry(idata->get_input("molecule")));
  std::list<std::pair<std::string, std::multimap<std::string, std::string>>> keys = idata->data();

  for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
    if (iter->first == "df-hf-opt") {
      std::shared_ptr<Opt<SCF<1>>> opt(new Opt<SCF<1>>(idata, iter->second, geom));
      for (int i = 0; i != 20; ++i)
        if (opt->next()) break;

      std::cout.rdbuf(backup_stream);
      return opt->geometry()->xyz();
    } else if (iter->first == "df-uhf-opt") {
      std::shared_ptr<Opt<UHF>> opt(new Opt<UHF>(idata, iter->second, geom));
      for (int i = 0; i != 20; ++i)
        if (opt->next()) break;

      std::cout.rdbuf(backup_stream);
      return opt->geometry()->xyz();
    } else if (iter->first == "df-rohf-opt") {
      std::shared_ptr<Opt<ROHF>> opt(new Opt<ROHF>(idata, iter->second, geom));
      for (int i = 0; i != 20; ++i)
        if (opt->next()) break;

      std::cout.rdbuf(backup_stream);
      return opt->geometry()->xyz();
    } else if (iter->first == "mp2-opt") {
      std::shared_ptr<Opt<MP2Grad>> opt(new Opt<MP2Grad>(idata, iter->second, geom));
      for (int i = 0; i != 20; ++i)
        if (opt->next()) break;

      std::cout.rdbuf(backup_stream);
      return opt->geometry()->xyz();
    } else if (iter->first == "casscf-opt") {
      std::shared_ptr<Opt<SuperCI>> opt(new Opt<SuperCI>(idata, iter->second, geom));
      for (int i = 0; i != 20; ++i)
        if (opt->next()) break;

      std::cout.rdbuf(backup_stream);
      return opt->geometry()->xyz();
    }
  }
  assert(false);
  return std::vector<double>();
}
std::vector<double> reference_scf_opt() {
  std::vector<double> out(6);
  out[2] = 1.749334;
  out[5] = 0.047492;
  return out;
}
std::vector<double> reference_scf_opt_cart() {
  std::vector<double> out(6);
  out[2] = 1.719396;
  out[5] = 0.016940;
  return out;
}
std::vector<double> reference_uhf_opt() {
  std::vector<double> out(6);
  out[2] = 1.800736;
  out[5] =-0.005890;
  return out;
}
std::vector<double> reference_rohf_opt() {
  std::vector<double> out(6);
  out[2] = 2.014163;
  out[5] =-0.084976;
  return out;
}
std::vector<double> reference_cas_act_opt() {
  std::vector<double> out(6);
  out[2] = 1.734489;
  out[5] =-0.003832;
  return out;
}
std::vector<double> reference_mp2_opt() {
  std::vector<double> out(6);
  out[2] = 1.932841;
  out[5] = 0.195930;
  return out;
}

BOOST_AUTO_TEST_SUITE(TEST_OPT)

BOOST_AUTO_TEST_CASE(DF_HF_Opt) {
    BOOST_CHECK(compare<std::vector<double>>(run_opt("hf_svp_dfhf_opt"),       reference_scf_opt(),      1.0e-4));
    BOOST_CHECK(compare<std::vector<double>>(run_opt("hf_svp_dfhf_opt_cart"),  reference_scf_opt_cart(), 1.0e-4));
    BOOST_CHECK(compare<std::vector<double>>(run_opt("oh_svp_uhf_opt"),        reference_uhf_opt(),      1.0e-4));
    BOOST_CHECK(compare<std::vector<double>>(run_opt("hc_svp_rohf_opt"),       reference_rohf_opt(),     1.0e-4));
}
BOOST_AUTO_TEST_CASE(MP2_Opt) {
    BOOST_CHECK(compare<std::vector<double>>(run_opt("hf_svp_mp2_opt"),        reference_mp2_opt(),      1.0e-4));
}
BOOST_AUTO_TEST_CASE(CASSCF_Opt) {
    BOOST_CHECK(compare<std::vector<double>>(run_opt("hf_svp_cas_act_opt"),    reference_cas_act_opt(),      1.0e-4));
}

BOOST_AUTO_TEST_SUITE_END()
