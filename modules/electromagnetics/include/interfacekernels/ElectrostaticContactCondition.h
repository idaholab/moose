//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADInterfaceKernel.h"
#include "Function.h"

/**
 *  This ADInterfaceKernel object calculates the electrostatic potential value
 *  and gradient relationship as a result of contact between two dissimilar,
 *  homogeneous materials.
 */
class ElectrostaticContactCondition : public ADInterfaceKernel
{
public:
  static InputParameters validParams();

  ElectrostaticContactCondition(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// Electrical conductivity property for the primary side
  const ADMaterialProperty<Real> & _conductivity_primary;

  /// Electrical conductivity property for the secondary side
  const ADMaterialProperty<Real> & _conductivity_secondary;

  /// Geometric mean of the hardness from both sides of the boundary, taken in as a material property
  const ADMaterialProperty<Real> & _mean_hardness;

  /// Mechanical pressure uniformly applied at the contact surface area (user-supplied for now)
  const Function & _mechanical_pressure;

  /// User-provided electrical contact conductance constant value
  const Real & _user_contact_conductance;

  /// Experimental proportional fit parameter for contact conductance parameter (set using Cincotti et al DOI:10.1002/aic.11102)
  const Real _alpha_electric;

  /// Experimental power fit parameter for contact conductance parameter (set using Cincotti et al DOI:10.1002/aic.11102)
  const Real _beta_electric;

  /// Check parameter for user-provided electrical contact conductance value
  bool _conductance_was_set;

  /// Check parameter for material-provided mean hardness value
  bool _mean_hardness_was_set;
};
