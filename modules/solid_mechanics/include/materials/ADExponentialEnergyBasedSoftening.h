//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADSmearedCrackSofteningBase.h"

/**
 * ExponentialEnergyBasedSoftening is a smeared crack softening model that
 * uses an exponential softening curve where the softening slope is 
 * dependent on each element size and the fracture toughness.
 */
class ADExponentialEnergyBasedSoftening : public ADSmearedCrackSofteningBase
{
public:
  static InputParameters validParams();

  ADExponentialEnergyBasedSoftening(const InputParameters & parameters);

  virtual void computeCrackingRelease(ADReal & stress,
                                      ADReal & stiffness_ratio,
                                      const ADReal & strain,
                                      const ADReal & crack_initiation_strain,
                                      const ADReal & crack_max_strain,
                                      const ADReal & cracking_stress,
                                      const ADReal & youngs_modulus, 
                                      const ADReal & poissons_ratio) override;

protected:
  /// Residual stress after full softening
  const Real & _residual_stress;

  /// Fracture toughness
  const Real & _fracture_toughness;
};
