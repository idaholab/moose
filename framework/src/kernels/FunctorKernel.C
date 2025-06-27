//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorKernel.h"

registerMooseObject("MooseApp", FunctorKernel);

InputParameters
FunctorKernel::validParams()
{
  InputParameters params = ADKernelValue::validParams();

  params.addClassDescription("Adds a term from a functor.");

  params.addRequiredParam<MooseFunctorName>("functor", "Functor to add");
  params.addRequiredParam<bool>(
      "functor_on_rhs",
      "If true, the functor to add is on the right hand side of the equation. By convention, all "
      "terms are moved to the left hand side, so if true, a factor of -1 is applied.");

  return params;
}

FunctorKernel::FunctorKernel(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _functor(getFunctor<ADReal>("functor")),
    _sign(getParam<bool>("functor_on_rhs") ? -1.0 : 1.0)
{
}

ADReal
FunctorKernel::precomputeQpResidual()
{
  const Moose::ElemQpArg space_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  return _sign * _functor(space_arg, Moose::currentState());
}
