//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeFiniteStrainElasticStress.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"
#include "StressUpdateBase.h"
#include "DamageBase.h"

/**
 * ADComputeMultipleInelasticStress computes the stress and a decomposition of the strain
 * into elastic and inelastic parts.  By default finite strains are assumed.
 *
 * The elastic strain is calculated by subtracting the computed inelastic strain
 * increment tensor from the mechanical strain tensor.  Mechanical strain is
 * considered as the sum of the elastic and inelastic (plastic, creep, ect) strains.
 *
 * This material is used to call the recompute iterative materials of a number
 * of specified inelastic models that inherit from ADStressUpdateBase.  It iterates
 * over the specified inelastic models until the change in stress is within
 * a user-specified tolerance, in order to produce the stress and the elastic and inelastic strains
 * for the time increment.
 */

class ADComputeMultipleInelasticStress : public ADComputeFiniteStrainElasticStress
{
public:
  static InputParameters validParams();

  ADComputeMultipleInelasticStress(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeQpStress() override;

  /**
    Compute the stress for the current QP, but do not rotate tensors from the
    intermediate configuration to the new configuration
   */
  virtual void computeQpStressIntermediateConfiguration();

  /**
   * Rotate _elastic_strain, _stress, and _inelastic_strain to the
   * new configuration.
   */
  virtual void finiteStrainRotation();

  /**
   * Given the _strain_increment[_qp], iterate over all of the user-specified
   * recompute materials in order to find an admissible stress (which is placed
   * into _stress[_qp]) and set of inelastic strains.
   * @param elastic_strain_increment The elastic part of _strain_increment[_qp] after the iterative
   * process has converged
   * @param combined_inelastic_strain_increment The inelastic part of _strain_increment[_qp] after
   * the iterative process has converged.  This is a weighted sum of all the inelastic strains
   * computed by all the plastic models during the Picard iterative scheme.  The weights are
   * dictated by the user using _inelastic_weights
   */
  virtual void updateQpState(ADRankTwoTensor & elastic_strain_increment,
                             ADRankTwoTensor & combined_inelastic_strain_increment);

  /**
   * An optimised version of updateQpState that gets used when the number
   * of plastic models is unity, or when we're cycling through models
   * Given the _strain_increment[_qp], find an admissible stress (which is
   * put into _stress[_qp]) and inelastic strain.
   * @param model_number Use this model number
   * @param elastic_strain_increment The elastic part of _strain_increment[_qp]
   * @param combined_inelastic_strain_increment The inelastic part of _strain_increment[_qp]
   */
  virtual void updateQpStateSingleModel(unsigned model_number,
                                        ADRankTwoTensor & elastic_strain_increment,
                                        ADRankTwoTensor & combined_inelastic_strain_increment);

  /**
   * Given a trial stress (_stress[_qp]) and a strain increment (elastic_strain_increment)
   * let the model_number model produce an admissible stress (gets placed back
   * in _stress[_qp]), and decompose the strain increment into an elastic part
   * (gets placed back into elastic_strain_increment) and an
   * inelastic part (inelastic_strain_increment).
   * @param model_number The inelastic model to use
   * @param elastic_strain_increment Upon input, this is the strain increment.
   * Upon output, it is the elastic part of the strain increment
   * @param inelastic_strain_increment The inelastic strain increment
   * corresponding to the supplied strain increment
   */
  virtual void computeAdmissibleState(unsigned model_number,
                                      ADRankTwoTensor & elastic_strain_increment,
                                      ADRankTwoTensor & inelastic_strain_increment);

  ///@{Input parameters associated with the recompute iteration to return the stress state to the yield surface
  const unsigned int _max_iterations;
  const Real _relative_tolerance;
  const Real _absolute_tolerance;
  const bool _internal_solve_full_iteration_history;
  ///@}

  /// after updateQpState, rotate the stress, elastic_strain, and inelastic_strain using _rotation_increment
  const bool _perform_finite_strain_rotations;

  /// The sum of the inelastic strains that come from the plastic models
  ADMaterialProperty<RankTwoTensor> & _inelastic_strain;

  /// old value of inelastic strain
  const MaterialProperty<RankTwoTensor> & _inelastic_strain_old;

  /// number of plastic models
  const unsigned _num_models;

  /// _inelastic_strain = sum_i (_inelastic_weights_i * inelastic_strain_from_model_i)
  const std::vector<Real> _inelastic_weights;

  /// whether to cycle through the models, using only one model per timestep
  const bool _cycle_models;

  MaterialProperty<Real> & _material_timestep_limit;

  /**
   * The user supplied list of inelastic models to use in the simulation
   *
   * Users should take care to list creep models first and plasticity
   * models last to allow for the case when a creep model relaxes the stress state
   * inside of the yield surface in an iteration.
   */
  std::vector<ADStressUpdateBase *> _models;

  /// is the elasticity tensor guaranteed to be isotropic?
  bool _is_elasticity_tensor_guaranteed_isotropic;

  /// Pointer to the damage model
  DamageBaseTempl<true> * _damage_model;

  RankTwoTensor _undamaged_stress_old;
};
