//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorTimeKernel.h"

#include "SystemBase.h"

registerADMooseObject("MooseApp", FVFunctorTimeKernel);

InputParameters
FVFunctorTimeKernel::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Residual contribution from time derivative of an AD functor (default "
                             "is the variable this kernel is acting upon if the 'functor' "
                             "parameter is not supplied)  for the finite volume method.");
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  params.addParam<MooseFunctorName>("functor",
                                    "The functor this kernel queries for the time derivative.");
  return params;
}

FVFunctorTimeKernel::FVFunctorTimeKernel(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _functor(isParamValid("functor")
                 ? static_cast<const Moose::FunctorBase<ADReal> &>(getFunctor<ADReal>("functor"))
                 : static_cast<Moose::FunctorBase<ADReal> &>(_var))
{
}

ADReal
FVFunctorTimeKernel::computeQpResidual()
{
  return _functor.dot(makeElemArg(_current_elem), determineState());
}
