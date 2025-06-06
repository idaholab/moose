//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GrayLambertSurfaceRadiationBase.h"

// Forward Declarations
class Function;

/**
 * ConstantViewFactorSurfaceRadiation computes radiative heat transfer between
 * side sets and the view factors are provided in the input file
 */
class ConstantViewFactorSurfaceRadiation : public GrayLambertSurfaceRadiationBase
{
public:
  static InputParameters validParams();

  ConstantViewFactorSurfaceRadiation(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  virtual std::vector<std::vector<Real>> setViewFactors() override;
};
