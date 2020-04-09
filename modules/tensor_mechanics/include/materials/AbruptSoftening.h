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

// Forward declaration

/**
 * AbruptSoftening is a smeared crack softening model that abruptly
 * drops the stress upon crack initiation. It is for use with
 * ComputeSmearedCrackingStress.
 */
class AbruptSoftening : public SmearedCrackSofteningBase
{
public:
  static InputParameters validParams();

  AbruptSoftening(const InputParameters & parameters);

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
};
