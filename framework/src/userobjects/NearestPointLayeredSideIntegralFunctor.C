//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPointLayeredSideIntegralFunctor.h"

registerMooseObject("MooseApp", NearestPointLayeredSideIntegralFunctor);

InputParameters
NearestPointLayeredSideIntegralFunctor::validParams()
{
  InputParameters params =
      NearestPointBase<LayeredSideIntegralFunctor, SideIntegralFunctorUserObject>::validParams();

  params.addClassDescription(
      "Computes layered side integrals of a functor nearest to a set of points.");

  return params;
}

NearestPointLayeredSideIntegralFunctor::NearestPointLayeredSideIntegralFunctor(
    const InputParameters & parameters)
  : NearestPointBase<LayeredSideIntegralFunctor, SideIntegralFunctorUserObject>(parameters)
{
}
