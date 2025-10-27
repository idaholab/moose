//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMultipleInelasticStressSmallStrain.h"

#include "StressUpdateBase.h"
#include "MooseException.h"
#include "MultipleInelasticStressHelper.h"
#include "libmesh/int_range.h"

registerMooseObject("SolidMechanicsApp", ComputeMultipleInelasticStressSmallStrain);

InputParameters
ComputeMultipleInelasticStressSmallStrain::validParams()
{
  InputParameters params = ComputeMultipleInelasticStressSmallStrainBase::validParams();
  params.addRequiredParam<std::vector<MaterialName>>(
      "inelastic_models",
      "The material objects to use to calculate stress and inelastic strains. "
      "Note: specify creep models first and plasticity models second.");
  params.addClassDescription("Compute state (stress and internal parameters such as plastic "
                             "strains and internal parameters) using an iterative process with "
                             "small strain formulation. Combinations of creep models and plastic "
                             "models may be used.");
  return params;
}

ComputeMultipleInelasticStressSmallStrain::ComputeMultipleInelasticStressSmallStrain(
    const InputParameters & parameters)
  : ComputeMultipleInelasticStressSmallStrainBase(parameters)
{
}

std::vector<MaterialName>
ComputeMultipleInelasticStressSmallStrain::getInelasticModelNames()
{
  return getParam<std::vector<MaterialName>>("inelastic_models");
}

void
ComputeMultipleInelasticStressSmallStrain::updateQpState(RankTwoTensor & elastic_strain,
                                                         RankTwoTensor & inelastic_strain)
{
  if (_internal_solve_full_iteration_history == true)
  {
    _console << std::endl
             << "iteration output for ComputeMultipleInelasticStressSmallStrain solve:"
             << " time=" << _t << " int_pt=" << _qp << std::endl;
  }

  Real l2norm_delta_stress;
  Real first_l2norm_delta_stress = 1.0;
  unsigned int counter = 0;

  std::vector<RankTwoTensor> inelastic_strain_increment;
  inelastic_strain_increment.resize(_num_models);

  for (const auto i_rmm : index_range(_models))
    inelastic_strain_increment[i_rmm].zero();

  RankTwoTensor stress_max, stress_min;

  do
  {
    for (const auto i_rmm : index_range(_models))
    {
      _models[i_rmm]->setQp(_qp);

      // Compute current elastic strain by subtracting all other inelastic strains
      // Start with total mechanical strain
      RankTwoTensor current_elastic_strain = _mechanical_strain[_qp] - _inelastic_strain_old[_qp];

      // Subtract all inelastic strain increments calculated so far except the current one
      for (const auto j_rmm : make_range(_num_models))
        if (i_rmm != j_rmm)
          current_elastic_strain -= inelastic_strain_increment[j_rmm];

      // Form the trial stress
      _stress[_qp] = _elasticity_tensor[_qp] * current_elastic_strain;

      // Compute strain increment for this iteration
      RankTwoTensor strain_increment = current_elastic_strain - _elastic_strain[_qp];

      // Given a trial stress and strain increment, let the model produce an admissible stress
      // and decompose the strain into elastic and inelastic parts
      computeAdmissibleState(i_rmm,
                             elastic_strain,
                             inelastic_strain_increment[i_rmm],
                             _consistent_tangent_operator[i_rmm]);

      // Update stress min/max for convergence checking
      MultipleInelasticStressHelper::updateStressMinMax(
          _stress[_qp], stress_max, stress_min, i_rmm == 0);
    }

    // Check convergence: once the change in stress is within tolerance after each model
    // consider the stress to be converged
    l2norm_delta_stress =
        MultipleInelasticStressHelper::computeStressDifferenceNorm(stress_max, stress_min);

    if (counter == 0 && l2norm_delta_stress > 0.0)
      first_l2norm_delta_stress = l2norm_delta_stress;

    if (_internal_solve_full_iteration_history == true)
    {
      _console << "stress iteration number = " << counter << "\n"
               << " relative l2 norm delta stress = "
               << (0 == first_l2norm_delta_stress ? 0
                                                  : l2norm_delta_stress / first_l2norm_delta_stress)
               << "\n"
               << " stress convergence relative tolerance = " << _relative_tolerance << "\n"
               << " absolute l2 norm delta stress = " << l2norm_delta_stress << "\n"
               << " stress convergence absolute tolerance = " << _absolute_tolerance << std::endl;
    }
    ++counter;
  } while (counter < _max_iterations && l2norm_delta_stress > _absolute_tolerance &&
           (l2norm_delta_stress / first_l2norm_delta_stress) > _relative_tolerance &&
           _num_models != 1);

  if (counter == _max_iterations && l2norm_delta_stress > _absolute_tolerance &&
      (l2norm_delta_stress / first_l2norm_delta_stress) > _relative_tolerance)
    throw MooseException(
        "Max stress iteration hit during ComputeMultipleInelasticStressSmallStrain solve!");

  // Combine inelastic strains from all models
  inelastic_strain = MultipleInelasticStressHelper::computeCombinedInelasticStrainIncrement(
      inelastic_strain_increment, _inelastic_weights, _num_models);

  // Add to old inelastic strain
  inelastic_strain += _inelastic_strain_old[_qp];

  // Compute final elastic strain
  elastic_strain = _mechanical_strain[_qp] - inelastic_strain;

  if (_fe_problem.currentlyComputingJacobian())
    computeQpJacobianMult();

  _material_timestep_limit[_qp] =
      MultipleInelasticStressHelper::computeMaterialTimestepLimit(_models, _num_models);
}
