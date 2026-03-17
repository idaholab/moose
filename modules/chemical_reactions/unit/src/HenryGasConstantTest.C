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

// Leave relative tolerance loose because the gold values were digitized from
// a publication
const double tol = 5.0e-2;

// Test of the HenryGasConstant UserObject for different temperatures
// This test is for FLiBe and FLiNaK and two different noble gases: helium and argon
// Gold values were digitized from:
//   K. Lee, et al., "Semi-empirical model for Henry's law constant of noble gases in molten salt",
//   Scientific Reports (2024) 14:12847, https://doi.org/10.1038/s41598-024-60006-9.

TEST_F(HenryGasConstantTest, test)
{

  // helium/FLiBe tests
  // Digitized value
  double gold = 4.8e-8 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_helium_flibe->henry(750.0) - gold), gold * tol);
  // Console output value
  gold = 4.71039676111e-07;
  EXPECT_LT(std::abs(_henry_helium_flibe->henry(750.0) - gold), libMesh::TOLERANCE);
  // Digitized value
  gold = 8.5e-8 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_helium_flibe->henry(900.0) - gold), gold * tol);
  // Console output value
  gold = 8.37095235109e-07;
  EXPECT_LT(std::abs(_henry_helium_flibe->henry(900.0) - gold), libMesh::TOLERANCE);
  // Digitized value
  gold = 1.4e-7 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_helium_flibe->henry(1100.0) - gold), gold * tol);
  // Console output value
  gold = 1.41185686539e-06;
  EXPECT_LT(std::abs(_henry_helium_flibe->henry(1100.0) - gold), libMesh::TOLERANCE);

  // argon/FLiBe tests
  // Digitized value
  gold = 4.3e-9 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_argon_flibe->henry(750.0) - gold), gold * tol);
  // Console output value
  gold = 4.25906423879e-08;
  EXPECT_LT(std::abs(_henry_argon_flibe->henry(750.0) - gold), libMesh::TOLERANCE);
  // Digitized value
  gold = 1.2e-8 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_argon_flibe->henry(900.0) - gold), gold * tol);
  // Console output value
  gold = 1.20121536439e-07;
  EXPECT_LT(std::abs(_henry_argon_flibe->henry(900.0) - gold), libMesh::TOLERANCE);
  // Digitized value
  gold = 3.1e-8 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_argon_flibe->henry(1100.0) - gold), gold * tol);
  // Console output value
  gold = 3.08312118085e-07;
  EXPECT_LT(std::abs(_henry_argon_flibe->henry(1100.0) - gold), libMesh::TOLERANCE);

  // helium/FLiNaK tests
  // Digitized value
  gold = 4.1e-8 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_helium_flinak->henry(750.0) - gold), gold * tol);
  // Console output value
  gold = 4.05786215977e-07;
  EXPECT_LT(std::abs(_henry_helium_flinak->henry(750.0) - gold), libMesh::TOLERANCE);
  // Digitized value
  gold = 9.1e-8 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_helium_flinak->henry(900.0) - gold), gold * tol);
  // Console output value
  gold = 8.95948557549e-07;
  EXPECT_LT(std::abs(_henry_helium_flinak->henry(900.0) - gold), libMesh::TOLERANCE);
  // Digitized value
  gold = 1.9e-7 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_helium_flinak->henry(1100.0) - gold), gold * tol);
  // Console output value
  gold = 1.84076108198e-06;
  EXPECT_LT(std::abs(_henry_helium_flinak->henry(1100.0) - gold), libMesh::TOLERANCE);

  // argon/FLiNaK tests
  // Digitized value
  gold = 2.5e-9 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(750.0) - gold), gold * tol);
  // Console output value
  gold = 2.53473212744e-08;
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(750.0) - gold), libMesh::TOLERANCE);
  // Digitized value
  gold = 1.1e-8 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(900.0) - gold), gold * tol);
  // Console output value
  gold = 1.05737661113e-07;
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(900.0) - gold), libMesh::TOLERANCE);
  // Digitized value
  gold = 4.0e-8 / (1e-6 * 101325);
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(1100.0) - gold), gold * tol);
  // Console output value
  gold = 3.87379501772e-07;
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(1100.0) - gold), libMesh::TOLERANCE);

  // argon/Custom tests (custom is actually FLiNaK)
  // Use a tighter tolerance because these should be identical
  gold = _henry_argon_custom->henry(750.0);
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(750.0) - gold), gold * libMesh::TOLERANCE);
  gold = _henry_argon_custom->henry(900.0);
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(900.0) - gold), gold * libMesh::TOLERANCE);
  gold = _henry_argon_custom->henry(1100.0);
  EXPECT_LT(std::abs(_henry_argon_flinak->henry(1100.0) - gold), gold * libMesh::TOLERANCE);
}
