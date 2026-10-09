//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPolynomialReaction.h"

registerKokkosResidualObject("KokkosTrainingApp", KokkosPolynomialReaction);

InputParameters
KokkosPolynomialReaction::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addClassDescription("Reaction term exp(u) - p(v) with a polynomial p of a coupled "
                             "variable v.");
  params.addRequiredCoupledVar("v", "The variable the polynomial is evaluated at");
  params.addRequiredParam<std::vector<Real>>(
      "coefficients", "Polynomial coefficients c_k of sum_k c_k v^k, starting from k = 0");
  return params;
}

KokkosPolynomialReaction::KokkosPolynomialReaction(const InputParameters & parameters)
  : KernelValue(parameters),
    _v(kokkosCoupledValue("v")),
    // Constructing from std::vector allocates and copies on both host and device
    _coefficients(getParam<std::vector<Real>>("coefficients"))
{
}
