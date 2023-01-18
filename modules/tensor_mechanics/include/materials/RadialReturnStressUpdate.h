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
#include "ADSingleVariableReturnMappingSolution.h"

/**
 * RadialReturnStressUpdate computes the radial return stress increment for
 * an isotropic elastic-viscoplasticity model after interating on the difference
 * between new and old trial stress increments.  This radial return mapping class
 * acts as a base class for the radial return creep and plasticity classes / combinations.
 * The stress increment computed by RadialReturnStressUpdate is used by
 * ComputeMultipleInelasticStress which computes the elastic stress for finite
 * strains.  This return mapping class is acceptable for finite strains but not
 * total strains.
 * This class is based on the Elasto-viscoplasticity algorithm in F. Dunne and N.
 * Petrinic's Introduction to Computational Plasticity (2004) Oxford University Press.
 */

template <bool is_ad>
class RadialReturnStressUpdateTempl : public StressUpdateBaseTempl<is_ad>,
                                      public SingleVariableReturnMappingSolutionTempl<is_ad>
{
public:
  static InputParameters validParams();

  RadialReturnStressUpdateTempl(const InputParameters & parameters);

  using Material::_current_elem;
  using Material::_dt;
  using Material::_q_point;
  using Material::_qp;

  enum class SubsteppingType
  {
    NONE,
    ERROR_BASED,
    INCREMENT_BASED
  };

  /**
   * A radial return (J2) mapping method is performed with return mapping
   * iterations.
   * @param strain_increment              Sum of elastic and inelastic strain increments
   * @param inelastic_strain_increment    Inelastic strain increment calculated by this class
   * @param rotation increment            Not used by this class
   * @param stress_new                    New trial stress from pure elastic calculation
   * @param stress_old                    Old state of stress
   * @param elasticity_tensor             Rank 4 C_{ijkl}, must be isotropic
   * @param elastic_strain_old            Old state of total elastic strain
   * @param compute_full_tangent_operator Flag currently unused by this class
   * @param tangent_operator              Currently a copy of the elasticity tensor in this class
   */

  virtual void updateState(
      GenericRankTwoTensor<is_ad> & strain_increment,
      GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
      const GenericRankTwoTensor<is_ad> & rotation_increment,
      GenericRankTwoTensor<is_ad> & stress_new,
      const RankTwoTensor & stress_old,
      const GenericRankFourTensor<is_ad> & elasticity_tensor,
      const RankTwoTensor & elastic_strain_old,
      bool compute_full_tangent_operator = false,
      RankFourTensor & tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor) override;

  virtual void updateStateSubstepInternal(
      GenericRankTwoTensor<is_ad> & /*strain_increment*/,
      GenericRankTwoTensor<is_ad> & /*inelastic_strain_increment*/,
      const GenericRankTwoTensor<is_ad> & /*rotation_increment*/,
      GenericRankTwoTensor<is_ad> & /*stress_new*/,
      const RankTwoTensor & /*stress_old*/,
      const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/,
      const RankTwoTensor & /*elastic_strain_old*/,
      unsigned int total_number_substeps,
      bool compute_full_tangent_operator = false,
      RankFourTensor & tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor);

  /**
   * Similar to the updateState function, this method updates the strain and stress for one substep
   */
  virtual void updateStateSubstep(
      GenericRankTwoTensor<is_ad> & /*strain_increment*/,
      GenericRankTwoTensor<is_ad> & /*inelastic_strain_increment*/,
      const GenericRankTwoTensor<is_ad> & /*rotation_increment*/,
      GenericRankTwoTensor<is_ad> & /*stress_new*/,
      const RankTwoTensor & /*stress_old*/,
      const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/,
      const RankTwoTensor & /*elastic_strain_old*/,
      bool compute_full_tangent_operator = false,
      RankFourTensor & tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor) override;

  virtual Real
  computeReferenceResidual(const GenericReal<is_ad> & effective_trial_stress,
                           const GenericReal<is_ad> & scalar_effective_inelastic_strain) override;

  virtual GenericReal<is_ad>
  minimumPermissibleValue(const GenericReal<is_ad> & /*effective_trial_stress*/) const override
  {
    return 0.0;
  }

  virtual GenericReal<is_ad>
  maximumPermissibleValue(const GenericReal<is_ad> & effective_trial_stress) const override;

  /**
   * Compute the limiting value of the time step for this material
   * @return Limiting time step
   */
  virtual Real computeTimeStepLimit() override;

  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return true; }

  /**
   * Radial return mapped models should be isotropic by default!
   */
  bool isIsotropic() override { return true; };

  /**
   * If substepping is enabled, calculate the number of substeps as a function
   * of the elastic strain increment guess and the maximum inelastic strain increment
   * ratio based on a user-specified tolerance.
   * @param strain_increment    When called, this is the elastic strain guess
   * @return                    The number of substeps required
   */
  virtual int
  calculateNumberSubsteps(const GenericRankTwoTensor<is_ad> & strain_increment) override;

  /**
   * Has the user requested usage of (possibly) implemented substepping capability for inelastic
   * models. Parent classes set this to false, but RadialReturn inelastic models have the
   * ability to implement substepping.
   */
  virtual bool substeppingCapabilityRequested() override
  {
    return _use_substepping != SubsteppingType::NONE;
  }

protected:
  virtual void initQpStatefulProperties() override;

  /**
   * Propagate the properties pertaining to this intermediate class.  This
   * is intended to be called from propagateQpStatefulProperties() in
   * classes that inherit from this one.
   * This is intentionally named uniquely because almost all models that derive
   * from this class have their own stateful properties, and this forces them
   * to define their own implementations of propagateQpStatefulProperties().
   */
  void propagateQpStatefulPropertiesRadialReturn();

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
   * Calculate the derivative of the strain increment with respect to the updated stress.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual Real computeStressDerivative(const Real /*effective_trial_stress*/, const Real /*scalar*/)
  {
    return 0.0;
  }

  /**
   * Perform any necessary steps to finalize state after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   */
  virtual void
  computeStressFinalize(const GenericRankTwoTensor<is_ad> & /*inelasticStrainIncrement*/)
  {
  }

  void outputIterationSummary(std::stringstream * iter_output,
                              const unsigned int total_it) override;

  /**
   * Calculate the tangent_operator.
   */
  void computeTangentOperator(Real /*effective_trial_stress*/,
                              RankTwoTensor & /*stress_new*/,
                              RankFourTensor & /*tangent_operator*/);
  /// 3 * shear modulus
  GenericReal<is_ad> _three_shear_modulus;

  GenericMaterialProperty<Real, is_ad> & _effective_inelastic_strain;
  const MaterialProperty<Real> & _effective_inelastic_strain_old;

  /// Stores the scalar effective inelastic strain increment from Newton iteration
  GenericReal<is_ad> _scalar_effective_inelastic_strain;

  /**
   * Maximum allowable scalar inelastic strain increment, used to control the
   * timestep size in conjunction with a user object
   */
  Real _max_inelastic_increment;

  /**
   * Used to calculate the number of substeps taken in the radial return algorithm,
   * when substepping is enabled, based on the elastic strain increment ratio
   * to the maximum inelastic increment
   */
  const Real _substep_tolerance;

  /**
   * Rank two identity tensor
   */
  const RankTwoTensor _identity_two;

  /**
   * Rank four symmetric identity tensor
   */
  const RankFourTensor _identity_symmetric_four;

  /**
   * Rank four deviatoric projection tensor
   */
  const RankFourTensor _deviatoric_projection_four;

  /// Debugging option to enable specifying instead of calculating strain
  const bool _apply_strain;

  /// Whether user has requested the use of substepping technique to improve convergence [make const later]
  SubsteppingType _use_substepping;

  /// Use adaptive substepping, cutting substep sizes until convergence is achieved
  const bool _adaptive_substepping;

  /// Maximum number of substeps. If the calculation results in a larger number, cut overall time step.
  const unsigned int _maximum_number_substeps;

  /// original timestep (to be restored after substepping is completed)
  Real _dt_original;
};

typedef RadialReturnStressUpdateTempl<false> RadialReturnStressUpdate;
typedef RadialReturnStressUpdateTempl<true> ADRadialReturnStressUpdate;
