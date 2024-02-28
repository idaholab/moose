//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrystalPlasticityStressUpdateBase.h"

/**
 * CrystalPlasticityTwinningKalidindiUpdate uses the multiplicative decomposition
 * of the deformation gradient, contributing shear due to twinning to the plastic
 * velocity gradient via the parent class.  If using this class in combination with
 * a dislocation slip model (the common case), make sure to provide that class
 * with the material property of the total twin volume fraction. Not all slip classes
 * are compatible with twinning.
 */

class CrystalPlasticityTwinningKalidindiUpdate : public CrystalPlasticityStressUpdateBase
{
public:
  static InputParameters validParams();

  CrystalPlasticityTwinningKalidindiUpdate(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void setInitialConstitutiveVariableValues() override;

  virtual void setSubstepConstitutiveVariableValues() override;

  virtual void updateSubstepConstitutiveVariableValues() override;

  /**
   * Despite the misnomer which results from the inheriting class structure,
   * Calculates the  twin shear increment following the phenomenological model
   * proposed in Kalidindi (2001) International Journal of Plasticity, 17(6)
   * pp. 837-860, eqn 18 and 19.
   */
  virtual bool calculateSlipRate() override;

  virtual void calculateConstitutiveSlipDerivative(std::vector<Real> & dslip_dtau) override;

  virtual void cacheStateVariablesBeforeUpdate() override;

  /**
   * Following the constitutive model contributions to plastic shear due to
   * deformation twinning propagation, following Kalidindi, S.R. Modeling
   * anisotropic strain hardening and deformation textures in low stacking fault
   * energy fcc metals. International Journal of Plasticity 17 (2001), 837-860,
   * eqn 18 and 19. Computes the value of the twin volume fraction increment for
   * each of the twin systems as a function of the plastic shear increment and
   * the characteristic twin shear value.
   */
  virtual void calculateStateVariableEvolutionRateComponent() override;

  virtual bool updateStateVariables() override;

  /**
   * Calculate the current value of the twin volume fraction from the incremented
   * twin volume fraction on each twin system and of the total twin volume
   * fraction across all twin systems in the crystal.
   * Also checks that the total twin volume fraction increment does not exceed user
   * set tolerances at each time step. Checks that the current total twin volume
   * fraction at each integration point remains under the the user-defined limit
   */
  bool calculateTwinVolumeFraction();

  /**
   * Calculates the resistance to twin propagation, following Kalidindi
   * IJP 17 (2001) 837-860 eqn 22.
   * Note that this value does not allow for softening due to de-twinning.
   */
  void calculateTwinResistance();

  virtual bool areConstitutiveStateVariablesConverged() override;

  ///@{Total volume fraction of twins across all twin systems
  MaterialProperty<Real> & _total_twin_volume_fraction;
  const MaterialProperty<Real> & _total_twin_volume_fraction_old;
  const Real _initial_total_twin_volume_fraction;
  ///@}

  ///@{Twin volume fraction per twin system
  MaterialProperty<std::vector<Real>> & _twin_volume_fraction;
  const MaterialProperty<std::vector<Real>> & _twin_volume_fraction_old;
  MaterialProperty<std::vector<Real>> & _twin_volume_fraction_increment;
  ///@}

  ///@{Power-law slip rate calculation coefficients, from Kalidindi IJP 17 (2001), 837-860
  const Real _reference_strain_rate;
  const Real _rate_sensitivity_exponent;
  ///@}

  ///@{Coefficients for twin dislocation propagation
  const Real _characteristic_twin_shear;
  const Real _twin_initial_lattice_friction;
  const Real _non_coplanar_coefficient_twin_hardening;
  const Real _coplanar_coefficient_twin_hardening;
  const Real _noncoplanar_exponent;
  const Real _limit_twin_volume_fraction;
  ///@}

  ///@{Stores the twin system resistance, twin volume fractions from the previous substep
  std::vector<Real> _previous_substep_twin_resistance;
  std::vector<Real> _previous_substep_twin_volume_fraction;
  // ///@}

  ///@{ Caching current twin resistance, twin volume fractions values before final update
  std::vector<Real> _twin_resistance_before_update;
  std::vector<Real> _twin_volume_fraction_before_update;
  ///@}
};
