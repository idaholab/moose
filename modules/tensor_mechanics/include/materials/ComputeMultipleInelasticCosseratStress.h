//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeMultipleInelasticStress.h"

/**
 * ComputeMultipleInelasticStress computes the stress, the consistent tangent
 * operator (or an approximation to it), and a decomposition of the strain
 * into elastic and inelastic parts.  By default finite strains are assumed.
 *
 * Cosserat couple-stress, and the cosserat version of the consistent
 * tangent operator are also computed, but only using Cosserat elasticity.
 *
 * The elastic strain is calculated by subtracting the computed inelastic strain
 * increment tensor from the mechanical strain tensor.  Mechanical strain is
 * considered as the sum of the elastic and inelastic (plastic, creep, ect) strains.
 *
 * This material is used to call the recompute iterative materials of a number
 * of specified inelastic models that inherit from StressUpdateBase.  It iterates
 * over the specified inelastic models until the change in stress is within
 * a user-specified tolerance, in order to produce the stress, the consistent
 * tangent operator and the elastic and inelastic strains for the time increment.
 */

class ComputeMultipleInelasticCosseratStress : public ComputeMultipleInelasticStress
{
public:
  static InputParameters validParams();

  ComputeMultipleInelasticCosseratStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpStress() override;
  virtual void computeQpJacobianMult() override;
  /**
   * The current Cosserat models do not know they might be using the
   * "host" version of the elasticity tensor during the
   * return-map process.  Therefore, they compute the incorrect
   * elastic/inelastic strain decomposition.  Overriding this
   * method allows the correction to be made.
   */
  virtual void computeAdmissibleState(unsigned model_number,
                                      RankTwoTensor & elastic_strain_increment,
                                      RankTwoTensor & inelastic_strain_increment,
                                      RankFourTensor & consistent_tangent_operator) override;

  /// The Cosserat curvature strain
  const MaterialProperty<RankTwoTensor> & _curvature;

  /// The Cosserat elastic flexural rigidity tensor
  const MaterialProperty<RankFourTensor> & _elastic_flexural_rigidity_tensor;

  /// the Cosserat couple-stress
  MaterialProperty<RankTwoTensor> & _couple_stress;

  /// the old value of Cosserat couple-stress
  const MaterialProperty<RankTwoTensor> & _couple_stress_old;

  /// derivative of couple-stress w.r.t. curvature
  MaterialProperty<RankFourTensor> & _Jacobian_mult_couple;

  /// Inverse of the elasticity tensor
  const MaterialProperty<RankFourTensor> & _compliance;
};
