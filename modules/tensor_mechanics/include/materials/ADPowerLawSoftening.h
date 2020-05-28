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
 * ADPowerLawSoftening is a smeared crack softening model that
 * uses a power law equation to soften the tensile response.
 * It is for use with ADComputeSmearedCrackingStress and uses
 * automatic differentiation.
 */
class ADPowerLawSoftening : public ADSmearedCrackSofteningBase
{
public:
  static InputParameters validParams();

  ADPowerLawSoftening(const InputParameters & parameters);

  virtual void computeCrackingRelease(ADReal & stress,
                                      ADReal & stiffness_ratio,
                                      const ADReal & strain,
                                      const ADReal & crack_initiation_strain,
                                      const ADReal & crack_max_strain,
                                      const ADReal & cracking_stress,
                                      const ADReal & youngs_modulus) override;

protected:
  /// Reduction factor applied to the initial stiffness each time a new crack initiates
  const Real & _stiffness_reduction;
};
