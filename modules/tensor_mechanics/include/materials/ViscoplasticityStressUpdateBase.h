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

template <bool is_ad>
class ViscoplasticityStressUpdateBaseTempl : public StressUpdateBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ViscoplasticityStressUpdateBaseTempl(const InputParameters & parameters);

  virtual Real computeTimeStepLimit() override;
  bool requiresIsotropicTensor() override { return true; }

  using StressUpdateBaseTempl<is_ad>::updateState;

protected:
  virtual void initQpStatefulProperties() override;

  virtual void propagateQpStatefulProperties() override;

  /**
   * Perform any necessary initialization before return mapping iterations
   * @param effective_trial_stress Effective trial stress
   * @param elasticityTensor     Elasticity tensor
   */
  virtual void computeStressInitialize(const GenericReal<is_ad> & /*effective_trial_stress*/,
                                       const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/)
  {
  }

  /**
   * Perform any necessary steps to finalize state after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   */
  virtual void
  computeStressFinalize(const GenericRankTwoTensor<is_ad> & /*plastic_strain_increment*/)
  {
  }

  void updateIntermediatePorosity(const GenericRankTwoTensor<is_ad> & elastic_strain_increment);

  /// String designating the base name of the total strain
  const std::string _total_strain_base_name;

  /// Material property for the total strain increment
  const GenericMaterialProperty<RankTwoTensor, is_ad> & _strain_increment;

  ///@{ Effective inelastic strain material property
  GenericMaterialProperty<Real, is_ad> & _effective_inelastic_strain;
  const MaterialProperty<Real> & _effective_inelastic_strain_old;
  ///@}

  ///@{ Creep strain material property
  GenericMaterialProperty<RankTwoTensor, is_ad> & _inelastic_strain;
  const MaterialProperty<RankTwoTensor> & _inelastic_strain_old;
  ///@}

  /// Max increment for inelastic strain
  Real _max_inelastic_increment;

  /// Container for the porosity calculated from all other intelastic models except the current model
  GenericReal<is_ad> _intermediate_porosity;

  /// Material property for the old porosity
  const MaterialProperty<Real> & _porosity_old;

  /// Flag to enable verbose output
  const bool _verbose;

  /// Initial porosity
  const Real _initial_porosity;

  enum class NegativeBehavior
  {
    ZERO,
    INITIAL_CONDITION,
    EXCEPTION
  };

  /// Enum for negative porosity handling
  const NegativeBehavior _negative_behavior;

  using StressUpdateBaseTempl<is_ad>::_dt;
  using StressUpdateBaseTempl<is_ad>::_name;
  using StressUpdateBaseTempl<is_ad>::_q_point;
  using StressUpdateBaseTempl<is_ad>::_qp;
  using StressUpdateBaseTempl<is_ad>::_base_name;
};

#define usingViscoplasticityStressUpdateBaseMembers                                                \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_dt;                                          \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_t;                                           \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_name;                                        \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_qp;                                          \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::isParamValid;                                 \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::paramError;                                   \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::paramWarning;                                 \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::updateState;                                  \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_console;                                     \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_q_point;                                     \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_verbose;                                     \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_intermediate_porosity;                       \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_effective_inelastic_strain;                  \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_effective_inelastic_strain_old;              \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_inelastic_strain;                            \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_inelastic_strain_old;                        \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::_porosity_old;                                \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::updateIntermediatePorosity;                   \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::computeStressFinalize;                        \
  using ViscoplasticityStressUpdateBaseTempl<is_ad>::computeStressInitialize

typedef ViscoplasticityStressUpdateBaseTempl<false> ViscoplasticityStressUpdateBase;
typedef ViscoplasticityStressUpdateBaseTempl<true> ADViscoplasticityStressUpdateBase;
