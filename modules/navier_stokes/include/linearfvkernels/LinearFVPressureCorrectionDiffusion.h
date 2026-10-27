//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAnisotropicDiffusion.h"

/**
 * Pressure correction diffusion kernel for the linear finite volume SIMPLE algorithm.
 */
class LinearFVPressureCorrectionDiffusion : public LinearFVAnisotropicDiffusion
{
public:
  static InputParameters validParams();

  LinearFVPressureCorrectionDiffusion(const InputParameters & params);
};
