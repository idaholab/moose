//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideIntegralFunctor.h"

registerMooseObject("MooseApp", LayeredSideIntegralFunctor);

InputParameters
LayeredSideIntegralFunctor::validParams()
{
  InputParameters params = LayeredSideIntegralBase<SideIntegralFunctorUserObject>::validParams();
  params.addClassDescription("Computes layered side integrals of a functor.");
  return params;
}

LayeredSideIntegralFunctor::LayeredSideIntegralFunctor(const InputParameters & parameters)
  : LayeredSideIntegralBase<SideIntegralFunctorUserObject>(parameters)
{
}
