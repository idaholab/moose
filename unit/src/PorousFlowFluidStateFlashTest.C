/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PorousFlowFluidStateFlashTest.h"

/**
 * Verify construction of the Rachford-Rice equation
 */
TEST_F(PorousFlowFluidStateFlashTest, rachfordRice)
{
  // Two fluid components
  Real vmf = 0.2316623869599;
  std::vector<Real> zi = {0.6};
  std::vector<Real> Ki = {1.338, 0.576};
  ABS_TEST("rachfordRice", _fp->rachfordRice(vmf, zi, Ki), 0.0, 1.0e-8);

  // Four fluid components
  vmf = 0.20329862165314910428;
  zi = {0.6, 0.01, 0.01};
  Ki = {1.338, 0.613, 0.222, 0.576};
  ABS_TEST("rachfordRice", _fp->rachfordRice(vmf, zi, Ki), 0.0, 1.0e-8);
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
  ABS_TEST("vaporMassFraction", _fp->vaporMassFraction(zi, Ki), 0.2316623869599, 1.0e-8);

  // Four fluid components
  zi = {0.6, 0.01, 0.01};
  Ki = {1.338, 0.613, 0.222, 0.576};
  ABS_TEST("rachfordRice", _fp->vaporMassFraction(zi, Ki), 0.20329862165314910428, 1.0e-8);
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

  ABS_TEST("rachfordRiceDeriv", _fp->rachfordRiceDeriv(vmf, zi, Ki), fd, 1.0e-8);

  // Four fluid components
  vmf = 0.6;
  zi = {0.6, 0.01, 0.01};
  Ki = {1.338, 0.613, 0.222, 0.576};

  rr1 = _fp->rachfordRice(vmf - dvmf, zi, Ki);
  rr2 = _fp->rachfordRice(vmf + dvmf, zi, Ki);
  fd = (rr2 - rr1) / (2.0 * dvmf);

  ABS_TEST("rachfordRiceDeriv", _fp->rachfordRiceDeriv(vmf, zi, Ki), fd, 1.0e-8);
}
