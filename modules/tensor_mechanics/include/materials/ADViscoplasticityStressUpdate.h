//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADStressUpdateBase.h"
#include "ADSingleVariableReturnMappingSolution.h"

template <ComputeStage>
class ADViscoplasticityStressUpdate;

declareADValidParams(ADViscoplasticityStressUpdate);

template <ComputeStage compute_stage>
class ADViscoplasticityStressUpdate : public ADStressUpdateBase<compute_stage>,
                                      public ADSingleVariableReturnMappingSolution<compute_stage>
{
public:
  ADViscoplasticityStressUpdate(const InputParameters & parameters);

  virtual void updateState(ADRankTwoTensor & strain_increment,
                           ADRankTwoTensor & inelastic_strain_increment,
                           const ADRankTwoTensor & rotation_increment,
                           ADRankTwoTensor & stress_new,
                           const RankTwoTensor & stress_old,
                           const ADRankFourTensor & elasticity_tensor,
                           const RankTwoTensor & elastic_strain_old) override;

  virtual Real computeReferenceResidual(const ADReal & effective_trial_stress,
                                        const ADReal & scalar_effective_inelastic_strain) override;

  virtual ADReal minimumPermissibleValue(const ADReal & effective_trial_stress) const override;

  virtual ADReal maximumPermissibleValue(const ADReal & effective_trial_stress) const override;

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
  virtual void computeStressInitialize(const ADReal & /*effective_trial_stress*/,
                                       const ADRankFourTensor & /*elasticity_tensor*/)
  {
  }

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
  virtual void computeStressFinalize(const ADRankTwoTensor & /*plastic_strain_increment*/) {}

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
  const MooseEnum _model;

  /// Enum to choose which pore shape model to use
  const MooseEnum _pore_shape;

  /// Pore shape factor depending on pore shape model
  const Real _pore_shape_factor;

  /// String designating the base name of the total strain
  const std::string _total_strain_base_name;

  /// Material property for the total strain increment
  const ADMaterialProperty(RankTwoTensor) & _strain_increment;

  ///@{ Effective inelastic strain material property
  ADMaterialProperty(Real) & _effective_inelastic_strain;
  const MaterialProperty<Real> & _effective_inelastic_strain_old;
  ///@}

  ///@{ Creep strain material property
  ADMaterialProperty(RankTwoTensor) & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;
  ///@}

  /// Max increment for inelastic strain
  Real _max_inelastic_increment;

  /// Exponent on the effective stress
  const Real _power;

  /// Power factor used for LPS model
  const Real _power_factor;

  /// Leading coefficient
  const ADMaterialProperty(Real) & _coefficient;

  /// Gauge stress
  ADMaterialProperty(Real) & _gauge_stress;

  /// Container for the porosity calculated from all other intelastic models except the current model
  ADReal _intermediate_porosity;

  /// Material property for the old porosity
  const MaterialProperty<Real> & _porosity_old;

  /// Flag to enable verbose output
  const bool _verbose;

  /// Container for hydrostatic stress
  ADReal _hydro_stress;

  /// Rank two identity tensor
  const RankTwoTensor _identity_two;

  /// Derivative of hydrostatic stress with respect to the stress tensor
  const RankTwoTensor _dhydro_stress_dsigma;

  /// Container for _derivative
  ADReal _derivative;

  usingStressUpdateBaseMembers;
  usingSingleVariableReturnMappingSolutionMembers;
};
