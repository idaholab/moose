//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVGradientMethod.h"

/**
 * Green-Gauss cell-centered gradient method for linear finite volume variables.
 */
class FVGreenGaussGradient : public FVGradientMethod
{
public:
  static InputParameters validParams();

  FVGreenGaussGradient(const InputParameters & params);

  Moose::FV::LinearFVGradientSchemeType schemeType() const override;
};
