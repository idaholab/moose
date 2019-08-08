//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StressUpdateBase.h"
#include "SingleVariableReturnMappingSolution.h"

class ViscoplasticityStressUpdate;

template <>
InputParameters validParams<ViscoplasticityStressUpdate>();

class ViscoplasticityStressUpdate : public StressUpdateBase,
                                    public SingleVariableReturnMappingSolution
{
public:
  ViscoplasticityStressUpdate(const InputParameters & parameters);

  virtual void updateState(RankTwoTensor & elastic_strain_increment,
                           RankTwoTensor & inelastic_strain_increment,
                           const RankTwoTensor & rotation_increment,
                           RankTwoTensor & stress_new,
                           const RankTwoTensor & stress_old,
                           const RankFourTensor & elasticity_tensor,
                           const RankTwoTensor & elastic_strain_old,
                           bool compute_full_tangent_operator,
                           RankFourTensor & tangent_operator) override;

  virtual Real computeReferenceResidual(const Real effective_trial_stress,
                                        const Real gauge_stress) override;

  virtual Real maximumPermissibleValue(const Real effective_trial_stress) const override;
  virtual Real minimumPermissibleValue(const Real effective_trial_stress) const override;

  /**
   * Compute the limiting value of the time step for this material
   * @return Limiting time step
   */
  virtual Real computeTimeStepLimit() override;

  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return true; }

  /// List of enums for choosing which viscoplastic model to use
  static MooseEnum getModelEnum();

  /// List of enums for choosing which pore shape model to use
  static MooseEnum getPoreShapeEnum();

protected:
  virtual void initQpStatefulProperties() override;

  virtual void propagateQpStatefulProperties() override;

  /**
   * Perform any necessary initialization before return mapping iterations
   * @param effective_trial_stress Effective trial stress
   * @param elasticityTensor     Elasticity tensor
   */
  virtual void computeStressInitialize(const Real /*effective_trial_stress*/,
                                       const RankFourTensor & /*elasticity_tensor*/)
  {
  }

  /**
   * Calculate the derivative of the strain increment with respect to the updated stress.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual Real computeStressDerivative(const Real /*effective_trial_stress*/, const Real /*scalar*/)
  {
    return 0.0;
  }

  /**
   * Compute an initial guess for the value of the scalar. For some cases, an
   * intellegent starting point can provide enhanced robustness in the Newton
   * iterations. This is also an opportunity for classes that derive from this
   * to perform initialization tasks.
   * @param effective_trial_stress Effective trial stress
   */
  virtual Real initialGuess(const Real effective_trial_stress) override;

  /**
   * Perform any necessary steps to finalize state after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   */
  virtual void computeStressFinalize(const RankTwoTensor & /*plastic_strain_increment*/) {}

  virtual Real computeResidual(const Real effective_trial_stress, const Real scalar) override;
  virtual Real computeDerivative(const Real /*effective_trial_stress*/,
                                 const Real /*scalar*/) override
  {
    return _derivative;
  }

  void outputIterationSummary(std::stringstream * iter_output,
                              const unsigned int total_it) override;

  virtual Real computeSwellingIncrement(const Real & /*hydrostatic_stress*/) { return 0.0; }

  Real computeH(const Real n, const Real & gauge_stress, const bool derivative = false);

  RankTwoTensor computeDGaugeDSigma(const Real & gauge_stress,
                                    const Real & equiv_stress,
                                    const RankTwoTensor & dev_stress,
                                    const RankTwoTensor & stress);

  void computeInelasticStrainIncrement(Real & gauge_stress,
                                       Real & dpsi_dgauge,
                                       RankTwoTensor & creep_strain_increment,
                                       const Real & equiv_stress,
                                       const RankTwoTensor & dev_stress,
                                       const RankTwoTensor & stress);

  /// Enum to choose which viscoplastic model to use
  const MooseEnum _model;

  /// Enum to choose which pore shape model to use
  const MooseEnum _pore_shape;

  /// Pore shape factor depending on pore shape model
  const Real _pore_shape_factor;

  /// String designating the base name of the total strain
  const std::string _total_strain_base_name;

  /// Material property for the total strain increment
  const MaterialProperty<RankTwoTensor> & _strain_increment;

  ///@{ Effective inelastic strain material property
  MaterialProperty<Real> & _effective_inelastic_strain;
  const MaterialProperty<Real> & _effective_inelastic_strain_old;
  ///@}

  ///@{ Creep strain material property
  MaterialProperty<RankTwoTensor> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;
  ///@}

  /// Max increment for inelastic strain
  Real _max_inelastic_increment;

  /// Exponent on the effective stress
  const Real _power;

  /// Power factor used for LPS model
  const Real _power_factor;

  /// Leading coefficient
  const MaterialProperty<Real> & _coefficient;

  /// Gauge stress
  MaterialProperty<Real> & _gauge_stress;

  /// Container for the porosity calculated from all other intelastic models except the current model
  Real _intermediate_porosity;

  /// Material property for the old porosity
  const MaterialProperty<Real> & _porosity_old;

  /// Flag to enable verbose output
  const bool _verbose;

  /// Container for hydrostatic stress
  Real _hydro_stress;

  /// Rank two identity tensor
  const RankTwoTensor _identity_two;

  /// Derivative of hydrostatic stress with respect to the stress tensor
  const RankTwoTensor _dhydro_stress_dsigma;

  /// Container for _derivative
  Real _derivative;
};
