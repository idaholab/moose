//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RandomICBase.h"

// Forward Declarations
class InputParameters;

template <typename T>
InputParameters validParams();

/**
 * VolumeWeightedWeibull generates a spatially randomized field that follows
 * a Weibull distribution weighted by the factor (V_ref/V_el)^1/m, where
 * V_ref is a reference volume from which the experimental data is obtained,
 * V_el is the finite element volume, and m is the Weibull modulus, to account
 * for the fact that larger material samples are more likely to contain defects.
 * This follows the approach of Strack, Leavy, and Brannon, IJNME (2015)
 * https://doi.org/10.1002/nme
 */
class VolumeWeightedWeibull : public RandomICBase
{
public:
  static InputParameters validParams();

  VolumeWeightedWeibull(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// The reference volume of the test specimen from which a median strength is obtained
  const Real _reference_volume;
  /// The Weibull modulus
  const Real _weibull_modulus;
  /// The median value of the strength for specimens having volume equal to the reference volume
  const Real _median;
};
