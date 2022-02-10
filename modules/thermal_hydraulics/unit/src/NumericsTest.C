//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "Numerics.h"
#include "THMTestUtils.h"

TEST(NumericsTest, test_absoluteFuzzyEqualVectors_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(1.0 + 1.0e-15, 2.0, 3.0);
  EXPECT_TRUE(THM::absoluteFuzzyEqualVectors(a, b));
}

TEST(NumericsTest, test_absoluteFuzzyEqualVectors_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(1.1, 2.0, 3.0);
  EXPECT_FALSE(THM::absoluteFuzzyEqualVectors(a, b));
}

TEST(NumericsTest, test_areParallelVectors_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(-2.0, -4.0, -6.0);
  EXPECT_TRUE(THM::areParallelVectors(a, b));
}

TEST(NumericsTest, test_areParallelVectors_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(2.0, 3.0, 1.0);
  EXPECT_FALSE(THM::areParallelVectors(a, b));
}

TEST(NumericsTest, test_haveSameDirection_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(2.0, 4.0, 6.0);
  EXPECT_TRUE(THM::haveSameDirection(a, b));
}

TEST(NumericsTest, test_haveSameDirection_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(-1.0, 2.0, 3.0);
  EXPECT_FALSE(THM::haveSameDirection(a, b));
}

TEST(NumericsTest, Reynolds)
{
  ABS_TEST(THM::Reynolds(0.1, 999, 0.5, 2e-2, 0.9), 1.11, 1e-13);
  ABS_TEST(THM::Reynolds(0.1, 999, -0.5, 2e-2, 0.9), 1.11, 1e-13);
}

TEST(NumericsTest, Prandtl) { ABS_TEST(THM::Prandtl(10, 0.1, 2), 0.5, 1e-13); }

TEST(NumericsTest, Grashof)
{
  ABS_TEST(THM::Grashof(0.1, 1, 2e-2, 999, 0.05, 9.81), 3.1329247392e3, 1e-13);
}

TEST(NumericsTest, Laplace) { ABS_TEST(THM::Laplace(0.001, 1, 9.81), 0.010096375546923, 1e-13); }

TEST(NumericsTest, viscosityNumber)
{
  ABS_TEST(THM::viscosityNumber(0.05, 0.02, 999, 2, 9.81), 0.062602188259, 1e-13);
}

TEST(NumericsTest, wallHeatTransferCoefficient)
{
  ABS_TEST(THM::wallHeatTransferCoefficient(2, 6, 3), 4, 1e-13);
}

TEST(NumericsTest, Dean) { ABS_TEST(THM::Dean(1, 4), 2, 1e-15); }

TEST(NumericsTest, vel)
{
  const Real arhoA = 1;
  const Real arhouA = 2;

  Real vel, dvel_darhoA, dvel_darhouA;
  THM::vel_from_arhoA_arhouA(arhoA, arhouA, vel, dvel_darhoA, dvel_darhouA);
  ABS_TEST(vel, 2., 1e-15);
  ABS_TEST(dvel_darhoA, -2, 1e-15);
  ABS_TEST(dvel_darhouA, 1., 1e-15);

  ABS_TEST(THM::dvel_darhoA(arhoA, arhouA), -2., 1e-15);
  ABS_TEST(THM::dvel_darhouA(arhoA), 1., 1e-15);
}

TEST(NumericsTest, rho)
{
  const Real arhoA = 2;
  const Real alpha = 0.1;
  const Real A = 1;

  Real rho, drho_darhoA, drho_dalpha;
  THM::rho_from_arhoA_alpha_A(arhoA, alpha, A, rho, drho_darhoA, drho_dalpha);
  ABS_TEST(rho, 20., 1e-15);
  ABS_TEST(drho_darhoA, 10., 1e-15);
  ABS_TEST(drho_dalpha, -200., 1e-13);
}

TEST(NumericsTest, specific_volume)
{
  const Real arhoA = 2;
  const Real A = 1;

  Real v, dv_drhoA;
  THM::v_from_rhoA_A(arhoA, A, v, dv_drhoA);
  ABS_TEST(v, 0.5, 1e-15);
  ABS_TEST(dv_drhoA, -0.25, 1e-15);

  const Real alpha = 0.1;
  Real dv_darhoA, dv_dalpha;
  THM::v_from_arhoA_alpha_A(arhoA, alpha, A, v, dv_darhoA, dv_dalpha);
  ABS_TEST(v, 0.05, 1e-15);
  ABS_TEST(dv_darhoA, -0.025, 1e-15);
  ABS_TEST(dv_dalpha, 0.5, 1e-13);

  const Real rho = 20;
  Real dv_drho;
  THM::v_from_rho(rho, v, dv_drho);
  ABS_TEST(v, 0.05, 1e-15);
  ABS_TEST(dv_darhoA, -0.025, 1e-15);

  ABS_TEST(THM::dv_dalpha_liquid(A, arhoA, true), 0.5, 1e-15);
  ABS_TEST(THM::dv_dalpha_liquid(A, arhoA, false), -0.5, 1e-15);

  ABS_TEST(THM::dv_darhoA(A, arhoA), -0.25, 1e-15);
}

TEST(NumericsTest, internal_energy)
{
  const Real arhoA = 2;
  const Real arhouA = 4;
  const Real arhoEA = 8;

  Real e, de_darhoA, de_darhouA, de_darhoEA;
  THM::e_from_arhoA_arhouA_arhoEA(arhoA, arhouA, arhoEA, e, de_darhoA, de_darhouA, de_darhoEA);
  ABS_TEST(e, 2., 1e-15);
  ABS_TEST(de_darhoA, 0., 1e-15);
  ABS_TEST(de_darhouA, -1., 1e-15);
  ABS_TEST(de_darhoEA, 0.5, 1e-15);

  const Real E = 10;
  const Real vel = 2;
  Real de_dE, de_dvel;
  THM::e_from_E_vel(E, vel, e, de_dE, de_dvel);
  ABS_TEST(e, 8., 1e-15);
  ABS_TEST(de_dE, 1., 1e-15);
  ABS_TEST(de_dvel, -2., 1e-15);

  ABS_TEST(THM::de_darhoA(arhoA, arhouA, arhoEA), 0., 1e-15);
  ABS_TEST(THM::de_darhouA(arhoA, arhouA), -1., 1e-15);
  ABS_TEST(THM::de_darhoEA(arhoA), 0.5, 1e-15);
}

TEST(NumericsTest, total_energy)
{
  const Real arhoA = 2.;
  const Real arhoEA = 8.;

  Real E, dE_darhoA, dE_darhoEA;
  THM::E_from_arhoA_arhoEA(arhoA, arhoEA, E, dE_darhoA, dE_darhoEA);
  ABS_TEST(E, 4., 1e-15);
  ABS_TEST(dE_darhoA, -2., 1e-15);
  ABS_TEST(dE_darhoEA, 0.5, 1e-15);

  const Real e = 4.;
  const Real vel = 2.;

  Real dE_de, dE_dvel;
  THM::E_from_e_vel(e, vel, E, dE_de, dE_dvel);

  ABS_TEST(E, 6., 1e-15);
  ABS_TEST(dE_de, 1., 1e-15);
  ABS_TEST(dE_dvel, 2., 1e-15);
}

TEST(NumericsTest, specific_enthalpy)
{
  const Real e = 6;
  const Real p = 4;
  const Real rho = 2;

  Real h, dh_de, dh_dp, dh_drho;
  THM::h_from_e_p_rho(e, p, rho, h, dh_de, dh_dp, dh_drho);
  ABS_TEST(h, 8., 1e-15);
  ABS_TEST(dh_de, 1., 1e-15);
  ABS_TEST(dh_dp, 0.5, 1e-15);
  ABS_TEST(dh_drho, -1., 1e-15);
}

TEST(NumericsTest, xlets)
{
  EXPECT_FALSE(THM::isInlet(2, 1));
  EXPECT_TRUE(THM::isInlet(2, -1));
  EXPECT_TRUE(THM::isInlet(-2, 1));
  EXPECT_FALSE(THM::isInlet(-2, -1));

  EXPECT_TRUE(THM::isOutlet(2, 1));
  EXPECT_FALSE(THM::isOutlet(2, -1));
  EXPECT_FALSE(THM::isOutlet(-2, 1));
  EXPECT_TRUE(THM::isOutlet(-2, -1));
}
