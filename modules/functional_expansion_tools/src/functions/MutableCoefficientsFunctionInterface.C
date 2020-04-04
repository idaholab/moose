//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MutableCoefficientsFunctionInterface.h"

InputParameters
MutableCoefficientsFunctionInterface::validParams()
{
  InputParameters params = MemoizedFunctionInterface::validParams();

  params += MutableCoefficientsInterface::validParams();

  params.addClassDescription("Interface based on MutableCoefficientsInterface for natively "
                             "supporting operations based on an array of coefficients");

  params.addParam<std::vector<Real>>("coefficients", "Coefficients required by the function.");

  return params;
}

MutableCoefficientsFunctionInterface::MutableCoefficientsFunctionInterface(
    const MooseObject * moose_object, const InputParameters & parameters)
  : MemoizedFunctionInterface(parameters),
    FunctionInterface(this),
    MutableCoefficientsInterface(moose_object, parameters)
{
  if (isParamValid("coefficients"))
    setCoefficients(getParam<std::vector<Real>>("coefficients")), enforceSize(true);
}

void
MutableCoefficientsFunctionInterface::coefficientsChanged()
{
  // The coefficients have changed, which invalidates the cache
  invalidateCache();
}
