//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmearedCrackSofteningBase.h"

/**
 * ExponentialSoftening is a smeared crack softening model that
 * uses an exponential softening curve. It is for use with
 * ComputeSmearedCrackingStress.
 */
class ExponentialSoftening : public SmearedCrackSofteningBase
{
public:
  static InputParameters validParams();

  ExponentialSoftening(const InputParameters & parameters);

  virtual void computeCrackingRelease(Real & stress,
                                      Real & stiffness_ratio,
                                      const Real strain,
                                      const Real crack_initiation_strain,
                                      const Real crack_max_strain,
                                      const Real cracking_stress,
                                      const Real youngs_modulus) override;

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
