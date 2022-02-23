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

class CrystalPlasticityHCPDislocationSlipBeyerleinUpdate;

/**
 * CrystalPlasticityHCPDislocationSlipBeyerleinUpdate computes the dislocation
 * forest evolution for the prismatic, pyramidal, and basal slip systems
 */

class CrystalPlasticityHCPDislocationSlipBeyerleinUpdate : public CrystalPlasticityStressUpdateBase
{
public:
  static InputParameters validParams();

  CrystalPlasticityHCPDislocationSlipBeyerleinUpdate(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void setInitialConstitutiveVariableValues() override;

  virtual void setSubstepConstitutiveVariableValues() override;

  virtual void updateSubstepConstitutiveVariableValues() override;

  virtual bool calculateSlipRate() override;

  virtual void
  calculateEquivalentSlipIncrement(RankTwoTensor & /*equivalent_slip_increment*/) override;

  virtual void calculateConstitutiveSlipDerivative(std::vector<Real> & dslip_dtau) override;

  virtual void cacheStateVariablesBeforeUpdate() override;

  /**
   * Following the constitutive model for HCP structures proposed by Beyerlein
   * and Tome. A dislocation-based constitutive law for pure Zr including
   * temperature effects. International Journal of Plasticity 24(2008)5, 867-895.
   * This class includes only the contributions for forest dislocation evolution
   * and the associated hardening contributions due to the forest and substructure
   * dislocations. Contributions for twin dislocations, as given in the paper
   * above, are included in a separate class.
   */
  virtual void calculateStateVariableEvolutionRateComponent() override;

  virtual bool updateStateVariables() override;

  /**
   * Computes the evolution increment of the forest (glide) dislocations on each
   * slip system, from equation 3.6 in Beyerlein and Tome IJP 24 (2008) 5, 867-895,
   * and using the increment approach shown in Ardeljan et al. Journal of Mechanics
   * and Physics of Solids 66 (2014) 16-31, equation 9.
   */
  void calculateForestDislocationEvolutionIncrement();

  /**
   * Computes the evolution of the total substructure dislocations,
   * using the nomenclature introduced in Capolungo et al. Materials Science
   * and Engineering A 513-514 (2009) 42-51, and the equation form given in
   * Zecevic et al. International Journal of Plasticity 84 (2016) 138-159.
   */
  void calculateSubstructureDensityEvolutionIncrement();

  /**
   * Calculate the current value of the incremented forest dislocation density
   * on each slip system
   */
  bool calculateForestDislocationDensity();

  /**
   * Calculates the current value of the incremented substructure dislocations
   */
  bool calculateSubstructureDislocationDensity();

  /**
   * Calculates the sum of the contribution of the initial slip resistance and the
   * forest and substructure dislocations to the slip resistance on each slip system,
   * equation 3.16 from Beyerlein and Tome IJP 24 (2008) 5, 867-895
   */
  virtual void calculateSlipResistance() override;

  /**
   * Calculates the influence of the grain size on the slip system resistance,
   * in the case where twin boundaries are not considered, following equation
   * 3.21 from Beyerlein and Tome IJP 24 (2008) 5, 867-895. These values are added
   * to the initial lattice friction vector. In the case where
   * twins are considered, this method should be overwritten to account for the
   * influence of the twin dislocations.
   */
  virtual void calculateGrainSizeResistance();

  /**
   * Determines if the state variables, e.g. defect densities, have converged
   * by comparing the change in the values over the iteration period.
   */
  virtual bool areConstitutiveStateVariablesConverged() override;

  /**
   * Determine if the substructure (debris) dislocation density values have converged
   */
  bool isSubstructureDislocationDensityConverged();

  /// Coupled temperature variable
  const VariableValue & _temperature;

  ///@{Slip forest dislocation densities
  MaterialProperty<std::vector<Real>> & _forest_dislocation_density;
  const MaterialProperty<std::vector<Real>> & _forest_dislocation_density_old;
  MaterialProperty<std::vector<Real>> & _forest_dislocation_increment;
  MaterialProperty<std::vector<Real>> & _forest_dislocations_removed_increment;
  const Real _initial_forest_dislocation_density;
  ///@}

  ///@{Sessile substructure dislocation density across all slip systems
  MaterialProperty<Real> & _total_substructure_density;
  const MaterialProperty<Real> & _total_substructure_density_old;
  MaterialProperty<Real> & _total_substructure_density_increment;
  const Real _initial_substructure_density;
  ///@}

  /**
   * The number of the different slip systems types to consider in the simulation,
   * which is dependent on the specific material. Types of slip systems, e.g.
   * prismatic, with two different burgers vectors, e.g. <a> and <c + a>, must be
   * considered as two separate slip system types.
   */
  const unsigned int _slip_system_modes;

  /// The number of slip systems per type, stored as a vector
  const std::vector<unsigned int> _number_slip_systems_per_mode;

  ///@{ Initial slip resistance vectors, with values arranaged as
  /// per each slip system mode
  const std::vector<Real> _lattice_friction;

  /// per each individual system system number
  DenseVector<Real> _initial_lattice_friction;
  ///@}

  ///@{Power-law slip rate calculation coefficients, from Wang et al IJP 49(2013)36-52
  const Real _reference_strain_rate;
  const Real _rate_sensitivity_exponent;
  ///@}

  ///@{Coefficients for slip dislocation evolution, Eqns 3.6 and 3.8
  const std::vector<Real> _burgers_vector;
  const std::vector<Real> _slip_generation_coefficient;
  const std::vector<Real> _slip_activation_energy;
  const std::vector<Real> _proportionality_factor;
  const Real _forest_interaction_coefficient;
  const Real _boltzman_constant;
  const Real _macro_applied_strain_rate;
  const Real _macro_reference_strain_rate;
  ///@}

  ///Forest hardening coefficients, Eqns. 3.19
  const std::vector<Real> _shear_modulus;

  /// Substructure (debris) dislocation generation rate coefficient
  const std::vector<Real> _substructure_rate_coefficient;

  /// Substructure (debris) Taylor law hardening coefficient
  const Real _substructure_hardening_coefficient;

  /**
   * Microscale Hall-Petch like coefficient, used to capture the effect of the
   * grain size on the slip system resistance in the case without twins (or before
   * twins have formed). Should not be confused with the engineering scale
   * Hall-Petch coefficient.
   */
  const std::vector<Real> _hallpetch_like_coefficient;

  /// Value of the grain size, either average or associated with a particular block
  const Real _grain_size;

  ///@{Stores the slip system resistance, dislocation densities from the previous substep
  std::vector<Real> _previous_substep_slip_resistance;
  std::vector<Real> _previous_substep_forest_dislocations;
  Real _previous_substep_total_substructure_density;
  ///@}

  ///@{ Caching current slip resistance, dislocation density values before final update
  std::vector<Real> _slip_resistance_before_update;
  std::vector<Real> _forest_dislocations_before_update;
  Real _total_substructure_density_before_update;
  ///@}

  /**
   * Flag to include the total twin volume fraction in the plastic velocity
   * gradient calculation, per Kalidindi IJP (2001).
   */
  const bool _include_twinning_in_Lp;

  /**
   * User-defined material property name for the total volume fraction of twins
   * in a twinning propagation constitutive model, when this class is used in
   * conjunction with the twinning propagation model.
   * Note that this value is the OLD material property and thus lags the current
   * value by a single timestep.
   */
  const MaterialProperty<Real> * const _twin_volume_fraction_total;
};
