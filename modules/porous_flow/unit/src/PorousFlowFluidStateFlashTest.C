//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateFlashTest.h"
#include "FluidPropertiesTestUtils.h"

/**
 * Verify construction of the Rachford-Rice equation
 */
TEST_F(PorousFlowFluidStateFlashTest, rachfordRice)
{
  // Two fluid components
  Real vmf = 0.2316623869599;
  std::vector<Real> zi = {0.6};
  std::vector<Real> Ki = {1.338, 0.576};
  ABS_TEST(_fp->rachfordRice(vmf, zi, Ki), 0.0, 1.0e-8);

  // Four fluid components
  vmf = 0.20329862165314910428;
  zi = {0.6, 0.01, 0.01};
  Ki = {1.338, 0.613, 0.222, 0.576};
  ABS_TEST(_fp->rachfordRice(vmf, zi, Ki), 0.0, 1.0e-8);
}

/**
 * Verify iterative solution of the Rachford-Rice equation to calculate the
 * vapor mass fraction
 */
TEST_F(PorousFlowFluidStateFlashTest, vaporMassFraction)
{
  // Test calculation using example data
  std::vector<Real> zi = {0.6};
  std::vector<Real> Ki = {1.338, 0.576};
  ABS_TEST(_fp->vaporMassFraction(zi, Ki), 0.2316623869599, 1.0e-8);

  // Four fluid components
  zi = {0.6, 0.01, 0.01};
  Ki = {1.338, 0.613, 0.222, 0.576};
  ABS_TEST(_fp->vaporMassFraction(zi, Ki), 0.20329862165314910428, 1.0e-8);
}

/**
 * Verify calculation of the derivative of the Rachford-Rice equation
 */
TEST_F(PorousFlowFluidStateFlashTest, rachfordRiceDeriv)
{
  // Two fluid components
  Real vmf = 0.25;
  const Real dvmf = 1.0e-8;
  std::vector<Real> zi = {0.6};
  std::vector<Real> Ki = {1.338, 0.576};

  Real rr1 = _fp->rachfordRice(vmf - dvmf, zi, Ki);
  Real rr2 = _fp->rachfordRice(vmf + dvmf, zi, Ki);
  Real fd = (rr2 - rr1) / (2.0 * dvmf);

  ABS_TEST(_fp->rachfordRiceDeriv(vmf, zi, Ki), fd, 1.0e-8);

  // Four fluid components
  vmf = 0.6;
  zi = {0.6, 0.01, 0.01};
  Ki = {1.338, 0.613, 0.222, 0.576};

  rr1 = _fp->rachfordRice(vmf - dvmf, zi, Ki);
  rr2 = _fp->rachfordRice(vmf + dvmf, zi, Ki);
  fd = (rr2 - rr1) / (2.0 * dvmf);

  ABS_TEST(_fp->rachfordRiceDeriv(vmf, zi, Ki), fd, 1.0e-8);
}
