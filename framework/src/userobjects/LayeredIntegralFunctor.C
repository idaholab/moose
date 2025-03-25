//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredIntegralFunctor.h"

registerMooseObject("MooseApp", LayeredIntegralFunctor);

InputParameters
LayeredIntegralFunctor::validParams()
{
  auto params = LayeredIntegralBase<ElementIntegralFunctorUserObject>::validParams();
  params.addClassDescription("Computes layered element integrals of a functor.");
  return params;
}

LayeredIntegralFunctor::LayeredIntegralFunctor(const InputParameters & parameters)
  : LayeredIntegralBase<ElementIntegralFunctorUserObject>(parameters)
{
}
