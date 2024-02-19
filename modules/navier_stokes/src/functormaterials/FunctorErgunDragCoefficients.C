//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorErgunDragCoefficients.h"

registerMooseObject("NavierStokesApp", FunctorErgunDragCoefficients);

InputParameters
FunctorErgunDragCoefficients::validParams()
{
  auto params = FunctorPebbleBedDragCoefficients<FunctorErgunDragCoefficients>::validParams();
  params.addClassDescription("Material providing linear and quadratic drag "
                             "coefficients based on the correlation developed by Ergun.");
  return params;
}

FunctorErgunDragCoefficients::FunctorErgunDragCoefficients(const InputParameters & parameters)
  : FunctorPebbleBedDragCoefficients<FunctorErgunDragCoefficients>(parameters)
{
}
