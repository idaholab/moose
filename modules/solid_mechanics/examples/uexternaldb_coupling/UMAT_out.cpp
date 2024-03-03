//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>

#include "omi_for_c.h"
#include "SMAAspUserSubroutines.h"
#include <SMAAspUserUtilities.h>

extern std::vector<std::map<std::pair<int, int>, double>> out_data;

extern "C" void FOR_NAME(umat, UMAT)(
    double * stress_ptr_,        // stress tensor at the beginning must be updated
    double * /*statev_ptr_*/,    // solution dependent state variables
    double * ddsdde_ptr_,        // Jacobian matrix of the constitutive model
    double * /*sse_ptr_*/,       // strain energy
    double * /*spd_ptr_*/,       // plastic dissipation
    double * /*scd_ptr_*/,       // creep dissipation
    double * /*rpl_ptr_*/,       // volumetric heat generation
    double * /*ddsddt_ptr_*/,    // variation of stress wrt temperature
    double * /*drplde_ptr_*/,    // variation of volumetric heat generation wrt strain increments
    double * /*drpldt_ptr_*/,    // variation of volumetric heat generation wrt temperature
    double * /*stran_ptr_*/,     // strain at the beginning
    double * dstran_ptr_,        // array of strain increment (total minus thermal)
    double * /*time_ptr_*/,      // value of the step time, value of the total time
    double * /*dtime_ptr_*/,     // time increment
    double * /*temp_ptr_*/,      // temperature at the start
    double * /*dtemp_ptr_*/,     // increment of temperature
    double * /*predef_ptr_*/,    // array of interpolated values of the predefine field variables at
                                 // this point at the start of the increment
    double * /*dpred_ptr_*/,     // increment of the predefine field of variable
    unsigned char * cmname_ptr_, // user define material name
    int * ndi_ptr_,              // number of stress direct component
    int * /*nshr_ptr_*/,         // number of engineering stress of the point
    int * ntens_ptr_,            // size of the strain or stress components \f$(NDI + NSHR)\f$
    int * /*nstatv_ptr_*/,       // number of solution dependent state variables
    double * props_ptr_,         // array of materials constants
    int * /*nprops_ptr_*/,       // number of materials constants
    double * /*coords_ptr_*/,    // coordinates of the point
    double * /*drot_ptr_*/,      // rotation increment matrix
    double * /*pnewdt_ptr_*/,    // ratio of suggested  new time increment being used
    double * /*celent_ptr_*/,    // characteristic element length
    double * /*dfgrd0_ptr_*/,    // deformation gradient at the beginning of the increment
    double * /*dfgrd1_ptr_*/,    // deformation gradient at the end
    int * noel_ptr_,             // number of element
    int * npt_ptr_,              // integration point number
    int * /*layer_ptr_*/,        // layer number (for composites)
    int * /*kspt_ptr_*/,         // section point number within the current layer
    int * /*kstep_ptr_*/,        // step number
    int * /*kinc_ptr_*/)         // increment number
{
  std::string mycmname; // assuming name are 80 character long
  for (int i = 0; i < 80 && cmname_ptr_[i] != ' '; ++i)
    mycmname += cmname_ptr_[i];

  auto myNthreads = CALL_NAME(getnumthreads, GETNUMTHREADS)();
  if (out_data.size() < std::size_t(myNthreads))
  {
    std::cerr << "out_data too small in " << mycmname << '\n';
    return;
  }

  auto myThreadID = CALL_NAME(get_thread_id, GET_THREAD_ID)();

  auto emod = props_ptr_[0];
  auto enu = props_ptr_[1];
  auto ebulk3 = emod / (1.0 - 2.0 * enu);
  auto eg2 = emod / (1.0 + enu);
  auto eg = eg2 / 2.0;
  auto elam = (ebulk3 - eg2) / 3.0;

  // elastic stiffness
  for (int k1 = 0; k1 < *ndi_ptr_; ++k1)
  {
    for (int k2 = 0; k2 < *ndi_ptr_; ++k2)
      ddsdde_ptr_[k1 * *ntens_ptr_ + k2] = elam;
    ddsdde_ptr_[k1 * *ntens_ptr_ + k1] += eg2;
  }
  for (int k1 = *ndi_ptr_; k1 < *ntens_ptr_; ++k1)
    ddsdde_ptr_[k1 * *ntens_ptr_ + k1] = eg;

  // calculate stress
  double l2 = 0.0;
  for (int k1 = 0; k1 < *ntens_ptr_; ++k1)
    for (int k2 = 0; k2 < *ntens_ptr_; ++k2)
    {
      auto inc = ddsdde_ptr_[k1 * *ntens_ptr_ + k2] * dstran_ptr_[k2];
      stress_ptr_[k1] += inc;
      l2 += inc * inc;
    }

  // store data
  out_data[myThreadID][std::make_pair(*noel_ptr_, *npt_ptr_)] = std::sqrt(l2);
}
