//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "HenryGasConstantTest.h"

#include <cmath>

const double tol = 1.0e-6;

// Test of the HenryGasConstant UserObject for different temperatures
// This test is for FLiBe and FLiNaK and two different noble gases: helium and argon
// Gold values were digitized from:
//   K. Lee, et al., "Semi-empirical model for Henry's law constant of noble gases in molten salt",
//   Scientific Reports (2024) 14:12847, https://doi.org/10.1038/s41598-024-60006-9.


TEST_F(HenryGasConstantTest, test)
{

   // helium/FLiBe tests
   EXPECT_NEAR(_henry_helium_flibe->henry(750.0), 4.8e-8/(1e-6*101325), tol);
   EXPECT_NEAR(_henry_helium_flibe->henry(900.0), 8.5e-8/(1e-6*101325), tol);
   EXPECT_NEAR(_henry_helium_flibe->henry(1100.0), 1.4e-7/(1e-6*101325), tol);

   // argon/FLiBe tests
   EXPECT_NEAR(_henry_argon_flibe->henry(750.0), 4.3e-9/(1e-6*101325), tol);
   EXPECT_NEAR(_henry_argon_flibe->henry(900.0), 1.2e-8/(1e-6*101325), tol);
   EXPECT_NEAR(_henry_argon_flibe->henry(1100.0), 3.1e-8/(1e-6*101325), tol);

   // helium/FLiNaK tests
   EXPECT_NEAR(_henry_helium_flinak->henry(750.0), 4.1e-8/(1e-6*101325), tol);
   EXPECT_NEAR(_henry_helium_flinak->henry(900.0), 9.1e-8/(1e-6*101325), tol);
   EXPECT_NEAR(_henry_helium_flinak->henry(1100.0), 1.9e-7/(1e-6*101325), tol);

   // argon/FLiNaK tests
   EXPECT_NEAR(_henry_argon_flinak->henry(750.0), 2.5e-9/(1e-6*101325), tol);
   EXPECT_NEAR(_henry_argon_flinak->henry(900.0), 1.1e-8/(1e-6*101325), tol);
   EXPECT_NEAR(_henry_argon_flinak->henry(1100.0), 4.0e-8/(1e-6*101325), tol);

}

