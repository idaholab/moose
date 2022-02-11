//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ViscoplasticityStressUpdateBase.h"
#include "ADSingleVariableReturnMappingSolution.h"

class ADViscoplasticityStressUpdate : public ADViscoplasticityStressUpdateBase,
                                      public ADSingleVariableReturnMappingSolution
{
public:
  static InputParameters validParams();

  ADViscoplasticityStressUpdate(const InputParameters & parameters);

  using ADViscoplasticityStressUpdateBase::updateState;

  virtual void updateState(ADRankTwoTensor & strain_increment,
                           ADRankTwoTensor & inelastic_strain_increment,
                           const ADRankTwoTensor & rotation_increment,
                           ADRankTwoTensor & stress_new,
                           const RankTwoTensor & stress_old,
                           const ADRankFourTensor & elasticity_tensor,
                           const RankTwoTensor & elastic_strain_old,
                           bool compute_full_tangent_operator = false,
                           RankFourTensor & tangent_operator = _identityTensor) override;

  virtual ADReal minimumPermissibleValue(const ADReal & effective_trial_stress) const override;

  virtual ADReal maximumPermissibleValue(const ADReal & effective_trial_stress) const override;

  virtual Real computeReferenceResidual(const ADReal & effective_trial_stress,
                                        const ADReal & scalar_effective_inelastic_strain) override;

protected:
  /**
   * Compute an initial guess for the value of the scalar. For some cases, an
   * intellegent starting point can provide enhanced robustness in the Newton
   * iterations. This is also an opportunity for classes that derive from this
   * to perform initialization tasks.
   * @param effective_trial_stress Effective trial stress
   */
  virtual ADReal initialGuess(const ADReal & effective_trial_stress) override;

  /**
   * Perform any necessary steps to finalize state after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   */
  virtual ADReal computeResidual(const ADReal & effective_trial_stress,
                                 const ADReal & scalar) override;

  virtual ADReal computeDerivative(const ADReal & /*effective_trial_stress*/,
                                   const ADReal & /*scalar*/) override
  {
    return _derivative;
  }

  void outputIterationSummary(std::stringstream * iter_output,
                              const unsigned int total_it) override;

  ADReal computeH(const Real n, const ADReal & gauge_stress, const bool derivative = false);

  ADRankTwoTensor computeDGaugeDSigma(const ADReal & gauge_stress,
                                      const ADReal & equiv_stress,
                                      const ADRankTwoTensor & dev_stress,
                                      const ADRankTwoTensor & stress);

  void computeInelasticStrainIncrement(ADReal & gauge_stress,
                                       ADReal & dpsi_dgauge,
                                       ADRankTwoTensor & creep_strain_increment,
                                       const ADReal & equiv_stress,
                                       const ADRankTwoTensor & dev_stress,
                                       const ADRankTwoTensor & stress);

  /// Enum to choose which viscoplastic model to use
  const enum class ViscoplasticityModel { LPS, GTN } _model;

  /// Enum to choose which pore shape model to use
  const enum class PoreShapeModel { SPHERICAL, CYLINDRICAL } _pore_shape;

  /// Pore shape factor depending on pore shape model
  const Real _pore_shape_factor;

  /// Exponent on the effective stress
  const Real _power;

  /// Power factor used for LPS model
  const Real _power_factor;

  /// Leading coefficient
  const ADMaterialProperty<Real> & _coefficient;

  /// Gauge stress
  ADMaterialProperty<Real> & _gauge_stress;

  /// Maximum ratio between the gauge stress and the equilvalent stress
  const Real _maximum_gauge_ratio;

  /// Minimum value of equivalent stress below which viscoplasticiy is not calculated
  const Real _minimum_equivalent_stress;

  /// Maximum value of equivalent stress above which an exception is thrown
  const Real _maximum_equivalent_stress;

  /// Container for hydrostatic stress
  ADReal _hydro_stress;

  /// Rank two identity tensor
  const RankTwoTensor _identity_two;

  /// Derivative of hydrostatic stress with respect to the stress tensor
  const RankTwoTensor _dhydro_stress_dsigma;

  /// Container for _derivative
  ADReal _derivative;
};
