//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "MathFVUtils.h"

/**
 * Interface function that holds the member variables and functions related to
 * the interpolation schemes used for diffusion problems.
 */
class FVDiffusionInterpolationInterface
{
public:
  static InputParameters validParams();
  FVDiffusionInterpolationInterface(const InputParameters & params);

protected:
  /// Decides which interpolation method should be used for the computation of
  /// the gradients within the face normal gradient.
  const Moose::FV::InterpMethod _var_interp_method;

  /// Just a convenience member for using skewness correction
  const bool _correct_skewness;
};
