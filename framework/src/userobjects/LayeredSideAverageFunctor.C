//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideAverageFunctor.h"

registerMooseObject("MooseApp", LayeredSideAverageFunctor);

InputParameters
LayeredSideAverageFunctor::validParams()
{
  InputParameters params = LayeredSideAverageBase<LayeredSideIntegralFunctor>::validParams();
  params.addClassDescription("Computes layered side averages of a functor.");
  return params;
}

LayeredSideAverageFunctor::LayeredSideAverageFunctor(const InputParameters & parameters)
  : LayeredSideAverageBase<LayeredSideIntegralFunctor>(parameters)
{
}
