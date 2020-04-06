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
 * PowerLawSoftening is a smeared crack softening model that
 * uses a power law equation to soften the tensile response.
 * It is for use with ComputeSmearedCrackingStress.
 */
class PowerLawSoftening : public SmearedCrackSofteningBase
{
public:
  static InputParameters validParams();

  PowerLawSoftening(const InputParameters & parameters);

  virtual void computeCrackingRelease(Real & stress,
                                      Real & stiffness_ratio,
                                      const Real strain,
                                      const Real crack_initiation_strain,
                                      const Real crack_max_strain,
                                      const Real cracking_stress,
                                      const Real youngs_modulus) override;

protected:
  /// Reduction factor applied to the initial stiffness each time a new crack initiates
  const Real & _stiffness_reduction;
};
