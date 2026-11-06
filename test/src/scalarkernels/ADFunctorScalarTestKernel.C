//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFunctorScalarTestKernel.h"

registerMooseObject("MooseTestApp", ADFunctorScalarTestKernel);

InputParameters
ADFunctorScalarTestKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params.addClassDescription("Subtracts a functor, evaluated at a point, from the residual of a "
                             "scalar variable.");
  params.addRequiredParam<MooseFunctorName>("functor", "The functor to subtract");
  params.addParam<Point>("point", Point(), "The point at which to evaluate the functor");
  return params;
}

ADFunctorScalarTestKernel::ADFunctorScalarTestKernel(const InputParameters & parameters)
  : ADScalarKernel(parameters),
    _functor(getFunctor<ADReal>("functor")),
    _point(getParam<Point>("point"))
{
}

ADReal
ADFunctorScalarTestKernel::computeQpResidual()
{
  const Moose::PointArg arg = {_point};
  return -_functor(arg, Moose::currentState());
}
