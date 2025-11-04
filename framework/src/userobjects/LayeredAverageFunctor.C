//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredAverageFunctor.h"

registerMooseObject("MooseApp", LayeredAverageFunctor);

InputParameters
LayeredAverageFunctor::validParams()
{
  InputParameters params =
      LayeredVolumeAverageBase<ElementIntegralFunctorUserObject>::validParams();
  params.addClassDescription("Computes layered side averages of a functor.");
  return params;
}

LayeredAverageFunctor::LayeredAverageFunctor(const InputParameters & parameters)
  : LayeredVolumeAverageBase<ElementIntegralFunctorUserObject>(parameters)
{
}
