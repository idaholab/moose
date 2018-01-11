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

#include "MutableCoefficientsFunctionInterface.h"

template <>
InputParameters
validParams<MutableCoefficientsFunctionInterface>()
{
  InputParameters params = validParams<MemoizedFunctionInterface>();

  params += validParams<MutableCoefficientsInterface>();

  params.addClassDescription("Interface based on MutableCoefficientsInterface for natively "
                             "supporting operations based on an array of coefficients");

  params.addParam<std::vector<Real>>("coefficients", "Coefficients required by the function.");

  return params;
}

MutableCoefficientsFunctionInterface::MutableCoefficientsFunctionInterface(
    const InputParameters & parameters)
  : MemoizedFunctionInterface(parameters),
    FunctionInterface(this),
    MutableCoefficientsInterface(parameters)
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
