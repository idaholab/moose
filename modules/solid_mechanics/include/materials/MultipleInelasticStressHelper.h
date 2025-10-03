//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "StressUpdateBase.h"

/**
 * Helper class containing shared logic for multiple inelastic stress calculations.
 * This allows code reuse between finite strain and small strain formulations.
 */
class MultipleInelasticStressHelper
{
public:
  /**
   * Compute the Jacobian multiplier using the elasticity tensor and consistent tangent operators
   * @param tangent_calculation_method The method for computing the tangent
   * @param elasticity_tensor The elasticity tensor
   * @param consistent_tangent_operator Vector of consistent tangent operators from each model
   * @param num_models Number of inelastic models
   * @param identity_symmetric_four The rank-4 symmetric identity tensor
   * @return The computed Jacobian multiplier
   */
  static RankFourTensor computeJacobianMult(
      TangentCalculationMethod tangent_calculation_method,
      const RankFourTensor & elasticity_tensor,
      const std::vector<RankFourTensor> & consistent_tangent_operator,
      unsigned int num_models,
      const RankFourTensor & identity_symmetric_four);

  /**
   * Compute the combined inelastic strain increment from individual model contributions
   * @param inelastic_strain_increment Vector of inelastic strain increments from each model
   * @param inelastic_weights Weights for combining the inelastic strains
   * @param num_models Number of inelastic models
   * @return The weighted sum of inelastic strain increments
   */
  static RankTwoTensor computeCombinedInelasticStrainIncrement(
      const std::vector<RankTwoTensor> & inelastic_strain_increment,
      const std::vector<Real> & inelastic_weights,
      unsigned int num_models);

  /**
   * Compute material timestep limit from all models
   * @param models Vector of stress update models
   * @param num_models Number of inelastic models
   * @return The computed timestep limit
   */
  static Real computeMaterialTimestepLimit(const std::vector<StressUpdateBase *> & models,
                                           unsigned int num_models);

  /**
   * Check convergence of the stress iteration
   * @param stress_max Maximum stress from current iteration
   * @param stress_min Minimum stress from current iteration
   * @param counter Current iteration number
   * @param max_iterations Maximum allowed iterations
   * @param relative_tolerance Relative convergence tolerance
   * @param absolute_tolerance Absolute convergence tolerance
   * @param first_l2norm_delta_stress L2 norm from first iteration (for relative check)
   * @param num_models Number of models (if 1, always converged)
   * @return true if converged, false otherwise
   */
  static bool checkConvergence(const RankTwoTensor & stress_max,
                               const RankTwoTensor & stress_min,
                               unsigned int counter,
                               unsigned int max_iterations,
                               Real relative_tolerance,
                               Real absolute_tolerance,
                               Real first_l2norm_delta_stress,
                               unsigned int num_models);

  /**
   * Update stress min/max for convergence checking
   * @param stress Current stress state
   * @param stress_max Maximum stress (updated in place)
   * @param stress_min Minimum stress (updated in place)
   * @param is_first_model True if this is the first model in the iteration
   */
  static void updateStressMinMax(const RankTwoTensor & stress,
                                 RankTwoTensor & stress_max,
                                 RankTwoTensor & stress_min,
                                 bool is_first_model);

  /**
   * Compute the L2 norm of the stress difference for convergence checking
   * @param stress_max Maximum stress
   * @param stress_min Minimum stress
   * @return L2 norm of the difference
   */
  static Real computeStressDifferenceNorm(const RankTwoTensor & stress_max,
                                          const RankTwoTensor & stress_min);
};
