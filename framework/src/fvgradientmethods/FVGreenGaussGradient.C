//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVGreenGaussGradient.h"

registerMooseObject("MooseApp", FVGreenGaussGradient);

InputParameters
FVGreenGaussGradient::validParams()
{
  InputParameters params = FVGradientMethod::validParams();
  params.addClassDescription("Green-Gauss cell-centered gradient method.");
  return params;
}

FVGreenGaussGradient::FVGreenGaussGradient(const InputParameters & params)
  : FVGradientMethod(params)
{
}

Moose::FV::LinearFVGradientSchemeType
FVGreenGaussGradient::schemeType() const
{
  return Moose::FV::LinearFVGradientSchemeType::GreenGauss;
}
