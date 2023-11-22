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

class CrystalPlasticityFCCDislocationLinkHuCocksUpdate;

/**
 * CrystalPlasticityFCCDislocationLinkHuCocksUpdate computes the athermal
 * pinning point density evolution due to dislocation glide, following
 * Hu and Cocks IJSS 78-79 (2016) 21-37.
 */

class CrystalPlasticityFCCDislocationLinkHuCocksUpdate : public CrystalPlasticityStressUpdateBase
{
public:
  static InputParameters validParams();

  CrystalPlasticityFCCDislocationLinkHuCocksUpdate(const InputParameters & parameters);

protected:
  /**
   * Calculate the slip system resistance due to precipitates and solutes, the
   * non-dislocation sources in the matrix, for the very first timestep
   */
  virtual void calculateInitialSlipResistance();

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
   * Calculates the evolution of the pinning points per plane, the inverse square
   * quantity of the dislocation link length, following Hu and Cocks, International
   * Journal of Solids and Structures 78-79 (2016) 21-37.
   */
  virtual void calculateStateVariableEvolutionRateComponent() override;

  virtual bool updateStateVariables() override;

  virtual void calculateConstitutiveCoplanarSlipIncrement();

  /**
   * Computes the evolution increment of the pinning points, on a per slip plane
   * basis, as function of the self-hardening plane and the other latent-hardening
   * slip planes. The pinning point increment due to latent-hardened planes is given
   * by equation 6b of Hu & Cocks, IJSS 78-79 (2016); the pinning point increment
   * due to the self-hardened plane is given by equation 7 in the same paper.
   */
  virtual void calculatePinningPointEvolutionIncrement();

  /**
   * Calculate the current value of the incremented pinning point density
   * on each slip plane (coplanar group) after checking the increment falls
   * within user-specified tolerances
   */
  bool calculatePinningPointDensity();

  /**
   * Sums the contributions from the solute slip resistance,
   * percipitate hardening, and the forest dislocation hardening, on a
   * per slip system basis. These different contributions are combined following
   * Hu et al, IJP 84 (2016) 203-223, Eq 2, in which the contributions of similar
   * number densities (less than three orders of magnitude) are combined with
   * a geometric mean, here the forest hardening and precipitate contriubtions.
   * Populations differing by more than three orders of magnitude are linearlly
   * added, here solute strengthening.
   */
  virtual void calculateSlipResistance() override;

  /**
   * Calculates the slip resistance due to the dislocation links (the
   * inverse of the square root of the pinning point desnity), following
   * equation 5 of Hu & Cocks, IJSS 78-19 (2016). This quantity is computed
   * on a per slip system basis. The pinning point density on a slip plane
   * is assumed to contribute equally to all coplanar slip systems.
   */
  virtual void calculateForestSlipResistance(std::vector<Real> & forest_hardening);

  /**
   * Calculates the slip resistance due to the solute concentration on a per
   * slip plane basis. The solute concentration, when coupled, is defined on
   * a per plane basis. Follows Eq 10 in Hu & Cocks, IJSS 78-79 (2016).
   */
  virtual void calculateSoluteResistance(std::vector<Real> & solute_hardening);

  /**
   * Calculates the slip system resistance due to percipitate number density, following
   * Eq 2 (middle term on rhs) and Eq 5 of Hu et al IJP 84 (2016) 203-223.
   */
  virtual void calculatePrecipitateResistance(std::vector<Real> & precipitate_hardening);

  /**
   * Determines if the dislocation densities have converged
   * by comparing the change in the values over the iteration period.
   */
  virtual bool areConstitutiveStateVariablesConverged() override;

  ///@{Pinning point (dislocation link) density quantities
  MaterialProperty<std::vector<Real>> & _pinning_point_density;
  const MaterialProperty<std::vector<Real>> & _pinning_point_density_old;
  MaterialProperty<std::vector<Real>> & _pinning_point_increment;
  const Real _initial_pinning_point_density;
  ///@}

  /**
   * Flag to include the solute hardening contribution in the slip system resistance
   * calculation
   */
  const bool _include_solute_hardening;

  /**
   * Solute atom number density, in 1/mm^3, computed by a separate material.
   *  Note that this value is the OLD material property and thus lags the current
   * value by a single timestep.
   */
  const MaterialProperty<Real> * const _solute_concentration;

  /**
   * Flag to include the precipitate hardening contribution in the slip system
   * resistance calculation
   */
  const bool _include_precipitate_hardening;

  /**
   * Precipitate number density, in 1/mm^3, as computed by a separate material.
   *  Note that this value is the OLD material property and thus lags the current
   * value by a single timestep.
   */
  const MaterialProperty<Real> * const _precipitate_density;

  /**
   * Mean precipitate radius, in mm, as computed by a separate material.
   *  Note that this value is the OLD material property and thus lags the current
   * value by a single timestep.
   */
  const MaterialProperty<Real> * const _precipitate_radius;

  ///@{Constants used to calculate the plastic slip increment
  /// reference slip rate increment
  const Real _gamma_reference;
  /// Strain rate sensitivity exponent
  const Real _p_exp;
  ///@}

  /**
   * Helper variable to store the coplanar sum of the slip_increment * substep_dt
   * vector values for use across the constitutive model calculations
   */
  MaterialProperty<std::vector<Real>> & _coplanar_constitutive_slip_increment;

  ///@{Calibration coefficients for the pinning points evolution terms
  const Real _self_pinpt_coeff;
  const Real _latent_pinpt_coeff;
  ///}

  ///@{Constants associated with slip system resistance
  const Real _forest_hardening_coeff;
  const Real _solute_hardening_coeff;
  const Real _precipitate_hardening_coeff;
  const Real _burgers_vector;
  const Real _shear_modulus;
  ///@}

  ///@{Stores the slip system resistance, dislocation densities from the previous substep
  std::vector<Real> _previous_substep_slip_resistance;
  std::vector<Real> _previous_substep_pinning_points;
  ///@}

  ///@{ Caching current slip resistance, pinning points density values before final update
  std::vector<Real> _slip_resistance_before_update;
  std::vector<Real> _pinning_points_before_update;
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
