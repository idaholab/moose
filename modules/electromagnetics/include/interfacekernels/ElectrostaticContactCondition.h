#pragma once

#include "ADInterfaceKernel.h"

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
  const MaterialProperty<Real> & _conductivity_master;

  /// Electrical conductivity property for the secondary side
  const MaterialProperty<Real> & _conductivity_neighbor;

  /// Geometric mean of the hardness from both sides of the boundary, taken in as a material property
  const ADMaterialProperty<Real> & _mean_hardness;

  /// Mechanical pressure uniformly applied at the contact surface area (user-supplied for now)
  const Real & _mechanical_pressure;

  /// User-provided electrical contact resistance constant value
  const Real & _user_contact_resistance;

  /// Experimental proportional fit parameter for contact resistance parameter (set using Cincotti et al DOI:10.1002/aic.11102)
  const Real _alpha_electric;

  /// Experimental power fit parameter for contact resistance parameter (set using Cincotti et al DOI:10.1002/aic.11102)
  const Real _beta_electric;

  /// Check parameter for user-provided electrical contact resistance value
  bool _resistance_was_set;

  /// Check parameter for material-provided mean hardness value
  bool _mean_hardness_was_set;
};
