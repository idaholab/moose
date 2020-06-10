//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADSmearedCrackSofteningBase.h"

/**
 * ExponentialSoftening is a smeared crack softening model that
 * uses an exponential softening curve. It is for use with
 * ADComputeSmearedCrackingStress and relies on automatic
 * differentiation.
 */
class ADExponentialSoftening : public ADSmearedCrackSofteningBase
{
public:
  static InputParameters validParams();

  ADExponentialSoftening(const InputParameters & parameters);

  virtual void computeCrackingRelease(ADReal & stress,
                                      ADReal & stiffness_ratio,
                                      const ADReal & strain,
                                      const ADReal & crack_initiation_strain,
                                      const ADReal & crack_max_strain,
                                      const ADReal & cracking_stress,
                                      const ADReal & youngs_modulus) override;

protected:
  /// Residual stress after full softening
  const Real & _residual_stress;

  /// Initial slope of the softening curve
  const Real & _alpha;

  /// Variable to track whether _alpha was set by the user
  const bool _alpha_set_by_user;

  /// Multiplier on alpha to determine the initial softening slope
  const Real & _beta;
};
