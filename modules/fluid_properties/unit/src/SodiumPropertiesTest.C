//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SodiumPropertiesTest.h"
#include "FluidPropertiesTestUtils.h"

/**
 * Verify calculation of Sodium Properties from ANL/RE-95/2 report
 * "Thermodynamic and Transport Properties of Sodium Liquid and Vapor"
 * from ANL Reactor Engineering Division.
 */
TEST_F(SodiumPropertiesTest, testSodium)
{
  Real T = 500; // K
  REL_TEST(_fp->k(T), 80.09125, 1.0E-5);
  REL_TEST(_fp->h(T), 381886.450000, 1.0E-5);
  REL_TEST(_fp->heatCapacity(T), 1345.578559, 1.0E-2);
  REL_TEST(_fp->temperature(_fp->h(T)), T, 1.0E-5);
  REL_TEST(_fp->rho(T), 896.9929544, 1E-05);
  REL_TEST(_fp->drho_dT(T), -0.224167872, 1E-05);
  REL_TEST(_fp->drho_dh(_fp->h(T)), -0.00016659590, 1E-2);
}
