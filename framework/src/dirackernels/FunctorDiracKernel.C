//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorDiracKernel.h"

registerMooseObject("MooseApp", FunctorDiracKernel);

InputParameters
FunctorDiracKernel::validParams()
{
  InputParameters params = ADDiracKernel::validParams();
  params.addRequiredParam<MooseFunctorName>("functor", "Source functor");
  params.addRequiredParam<Point>("point", "Source point");
  params.addClassDescription("Computes a dirac source using a functor.");
  return params;
}

FunctorDiracKernel::FunctorDiracKernel(const InputParameters & parameters)
  : ADDiracKernel(parameters), _functor(getFunctor<ADReal>("functor")), _p(getParam<Point>("point"))
{
}

void
FunctorDiracKernel::addPoints()
{
  addPoint(_p);
}

ADReal
FunctorDiracKernel::computeQpResidual()
{
  mooseAssert(_current_point == _p, "Current point must be user-provided point");
  const Moose::ElemQpArg space_arg = {_current_elem, _qp, _qrule, _current_point};
  return -_test[_i][_qp] * _functor(space_arg, Moose::currentState());
}
