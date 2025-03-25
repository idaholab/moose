//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserForcingFunctorNodalKernel.h"
#include "MooseFunctor.h"

registerMooseObject("MooseApp", UserForcingFunctorNodalKernel);

InputParameters
UserForcingFunctorNodalKernel::validParams()
{
  InputParameters params = ADNodalKernel::validParams();
  params.addClassDescription(
      "Residual contribution to an ODE from a source functor acting at nodes.");
  params.addRequiredParam<MooseFunctorName>("functor", "The forcing functor");
  return params;
}

UserForcingFunctorNodalKernel::UserForcingFunctorNodalKernel(const InputParameters & parameters)
  : ADNodalKernel(parameters), _functor(getFunctor<ADReal>("functor"))
{
}

ADReal
UserForcingFunctorNodalKernel::computeQpResidual()
{
  return -_functor(nodeArg(), determineState());
}
