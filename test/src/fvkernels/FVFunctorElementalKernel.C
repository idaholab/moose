//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorElementalKernel.h"

registerMooseObject("MooseTestApp", FVFunctorElementalKernel);

InputParameters
FVFunctorElementalKernel::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addRequiredParam<MooseFunctorName>(
      "functor_name",
      "The name of the functor (material property for example) that will provide "
      "the residual");
  return params;
}

FVFunctorElementalKernel::FVFunctorElementalKernel(const InputParameters & params)
  : FVElementalKernel(params), _functor(getFunctor<ADReal>("functor_name"))
{
}

ADReal
FVFunctorElementalKernel::computeQpResidual()
{
  return _functor(makeElemArg(_current_elem), Moose::currentState());
}
