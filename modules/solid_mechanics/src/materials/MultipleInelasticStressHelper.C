//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultipleInelasticStressHelper.h"
#include "MooseUtils.h"
#include "libmesh/int_range.h"

RankFourTensor
MultipleInelasticStressHelper::computeJacobianMult(
    TangentCalculationMethod tangent_calculation_method,
    const RankFourTensor & elasticity_tensor,
    const std::vector<RankFourTensor> & consistent_tangent_operator,
    unsigned int num_models,
    const RankFourTensor & identity_symmetric_four)
{
  if (tangent_calculation_method == TangentCalculationMethod::ELASTIC)
    return elasticity_tensor;
  else if (tangent_calculation_method == TangentCalculationMethod::PARTIAL)
  {
    RankFourTensor A = identity_symmetric_four;
    for (const auto i_rmm : make_range(num_models))
      A += consistent_tangent_operator[i_rmm];
    mooseAssert(A.isSymmetric(), "Tangent operator isn't symmetric");
    return A.invSymm() * elasticity_tensor;
  }
  else // FULL
  {
    const RankFourTensor E_inv = elasticity_tensor.invSymm();
    RankFourTensor result = consistent_tangent_operator[0];
    for (const auto i_rmm : make_range(1u, num_models))
      result = consistent_tangent_operator[i_rmm] * E_inv * result;
    return result;
  }
}

RankTwoTensor
MultipleInelasticStressHelper::computeCombinedInelasticStrainIncrement(
    const std::vector<RankTwoTensor> & inelastic_strain_increment,
    const std::vector<Real> & inelastic_weights,
    unsigned int num_models)
{
  RankTwoTensor combined;
  combined.zero();
  for (const auto i_rmm : make_range(num_models))
    combined += inelastic_weights[i_rmm] * inelastic_strain_increment[i_rmm];
  return combined;
}

Real
MultipleInelasticStressHelper::computeMaterialTimestepLimit(
    const std::vector<StressUpdateBase *> & models, unsigned int num_models)
{
  Real limit = 0.0;
  for (const auto i_rmm : make_range(num_models))
    limit += 1.0 / models[i_rmm]->computeTimeStepLimit();

  if (MooseUtils::absoluteFuzzyEqual(limit, 0.0))
    return std::numeric_limits<Real>::max();
  else
    return 1.0 / limit;
}

bool
MultipleInelasticStressHelper::checkConvergence(const RankTwoTensor & stress_max,
                                                const RankTwoTensor & stress_min,
                                                unsigned int counter,
                                                unsigned int max_iterations,
                                                Real relative_tolerance,
                                                Real absolute_tolerance,
                                                Real first_l2norm_delta_stress,
                                                unsigned int num_models)
{
  // Single model is always converged
  if (num_models == 1)
    return true;

  Real l2norm_delta_stress = computeStressDifferenceNorm(stress_max, stress_min);

  // Check convergence criteria
  if (counter >= max_iterations)
    return false;

  if (l2norm_delta_stress <= absolute_tolerance)
    return true;

  if (first_l2norm_delta_stress > 0.0 &&
      (l2norm_delta_stress / first_l2norm_delta_stress) <= relative_tolerance)
    return true;

  return false;
}

void
MultipleInelasticStressHelper::updateStressMinMax(const RankTwoTensor & stress,
                                                  RankTwoTensor & stress_max,
                                                  RankTwoTensor & stress_min,
                                                  bool is_first_model)
{
  if (is_first_model)
  {
    stress_max = stress;
    stress_min = stress;
  }
  else
  {
    for (const auto i : make_range(Moose::dim))
      for (const auto j : make_range(Moose::dim))
      {
        if (stress(i, j) > stress_max(i, j))
          stress_max(i, j) = stress(i, j);
        else if (stress_min(i, j) > stress(i, j))
          stress_min(i, j) = stress(i, j);
      }
  }
}

Real
MultipleInelasticStressHelper::computeStressDifferenceNorm(const RankTwoTensor & stress_max,
                                                           const RankTwoTensor & stress_min)
{
  return (stress_max - stress_min).L2norm();
}
