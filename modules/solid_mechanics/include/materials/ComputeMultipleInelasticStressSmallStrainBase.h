//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeStressBase.h"
#include "GuaranteeConsumer.h"
#include "DamageBase.h"
#include "StressUpdateBase.h"
#include "MultipleInelasticStressHelper.h"

/**
 * ComputeMultipleInelasticStressSmallStrainBase computes the stress, the consistent tangent
 * operator (or an approximation to it), and a decomposition of the strain
 * into elastic and inelastic parts using small strain formulation.
 *
 * The elastic strain is calculated by subtracting the computed inelastic strain
 * from the mechanical strain tensor. Mechanical strain is considered as the sum
 * of the elastic and inelastic (plastic, creep, etc) strains.
 *
 * This material is used to call the recompute iterative materials of a number
 * of specified inelastic models that inherit from StressUpdateBase. It iterates
 * over the specified inelastic models until the change in stress is within
 * a user-specified tolerance, in order to produce the stress, the consistent
 * tangent operator and the elastic and inelastic strains for the time increment.
 *
 * This class uses small strain (total) formulation, in contrast to
 * ComputeMultipleInelasticStressBase which uses finite strain (incremental) formulation.
 */

class ComputeMultipleInelasticStressSmallStrainBase : public ComputeStressBase,
                                                      public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  ComputeMultipleInelasticStressSmallStrainBase(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual std::vector<MaterialName> getInelasticModelNames() = 0;

  virtual void initQpStatefulProperties() override;

  virtual void computeQpStress() override;

  /**
   * Given the current strain, iterate over all of the user-specified
   * recompute materials in order to find an admissible stress (which is placed
   * into _stress[_qp]) and set of inelastic strains, as well as the tangent operator
   * (which is placed into _Jacobian_mult[_qp]).
   * @param elastic_strain The elastic strain after the iterative process has converged
   * @param inelastic_strain The inelastic strain after the iterative process has converged
   */
  virtual void updateQpState(RankTwoTensor & elastic_strain, RankTwoTensor & inelastic_strain) = 0;

  /**
   * An optimised version of updateQpState that gets used when the number
   * of plastic models is unity, or when we're cycling through models
   * Given the current strain, find an admissible stress (which is
   * put into _stress[_qp]) and inelastic strain, as well as the tangent operator
   * (which is placed into _Jacobian_mult[_qp])
   * @param model_number Use this model number
   * @param elastic_strain The elastic strain
   * @param inelastic_strain The inelastic strain
   */
  virtual void updateQpStateSingleModel(unsigned model_number,
                                        RankTwoTensor & elastic_strain,
                                        RankTwoTensor & inelastic_strain);

  /**
   * Using _elasticity_tensor[_qp] and the consistent tangent operators,
   * _consistent_tangent_operator[...] computed by the inelastic models,
   * compute _Jacobian_mult[_qp]
   */
  virtual void computeQpJacobianMult();

  /**
   * Given a trial stress (_stress[_qp]) and the current mechanical strain
   * let the model_number model produce an admissible stress (gets placed back
   * in _stress[_qp]), and compute the elastic and inelastic strains,
   * as well as the consistent_tangent_operator
   * @param model_number The inelastic model to use
   * @param elastic_strain The elastic strain (computed)
   * @param inelastic_strain The inelastic strain (computed)
   * @param consistent_tangent_operator The consistent tangent operator
   */
  virtual void computeAdmissibleState(unsigned model_number,
                                      RankTwoTensor & elastic_strain,
                                      RankTwoTensor & inelastic_strain,
                                      RankFourTensor & consistent_tangent_operator);

  ///@{Input parameters associated with the recompute iteration to return the stress state to the yield surface
  const unsigned int _max_iterations;
  const Real _relative_tolerance;
  const Real _absolute_tolerance;
  const bool _internal_solve_full_iteration_history;
  ///@}

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// Old value of mechanical strain
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;

  /// The sum of the inelastic strains that come from the plastic models
  MaterialProperty<RankTwoTensor> & _inelastic_strain;

  /// old value of inelastic strain
  const MaterialProperty<RankTwoTensor> & _inelastic_strain_old;

  /// Old value of stress
  const MaterialProperty<RankTwoTensor> & _stress_old;

  /// what sort of Tangent operator to calculate
  const enum class TangentOperatorEnum { elastic, nonlinear } _tangent_operator_type;

  /// number of plastic models
  unsigned _num_models;

  /// Flags to compute tangent during updateState call
  std::vector<bool> _tangent_computation_flag;

  /// Calculation method for the tangent modulus
  TangentCalculationMethod _tangent_calculation_method;

  /// _inelastic_strain = sum_i (_inelastic_weights_i * inelastic_strain_from_model_i)
  std::vector<Real> _inelastic_weights;

  /// the consistent tangent operators computed by each plastic model
  std::vector<RankFourTensor> _consistent_tangent_operator;

  /// whether to cycle through the models, using only one model per timestep
  const bool _cycle_models;

  MaterialProperty<Real> & _material_timestep_limit;

  /**
   * Rank four symmetric identity tensor
   */
  const RankFourTensor _identity_symmetric_four;

  /**
   * The user supplied list of inelastic models to use in the simulation
   *
   * Users should take care to list creep models first and plasticity
   * models last to allow for the case when a creep model relaxes the stress state
   * inside of the yield surface in an iteration.
   */
  std::vector<StressUpdateBase *> _models;

  /// is the elasticity tensor guaranteed to be isotropic?
  bool _is_elasticity_tensor_guaranteed_isotropic;

  /// are all inelastic models inherently isotropic? (not the case for e.g. weak plane plasticity models)
  bool _all_models_isotropic;

  /// Pointer to the damage model
  DamageBaseTempl<false> * _damage_model;

  RankTwoTensor _undamaged_stress_old;
};
