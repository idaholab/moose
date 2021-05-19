//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "LayeredSideAverage.h"

/**
 * This UserObject computes side averages of a flux storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 */
class LayeredSideDiffusiveFluxAverage : public LayeredSideAverage
{
public:
  static InputParameters validParams();

  LayeredSideDiffusiveFluxAverage(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  std::string _diffusivity;
  const MaterialProperty<Real> & _diffusion_coef;
};
