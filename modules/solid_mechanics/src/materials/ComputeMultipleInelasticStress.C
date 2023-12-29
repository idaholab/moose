//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMultipleInelasticStress.h"

#include "StressUpdateBase.h"
#include "MooseException.h"
#include "DamageBase.h"
#include "libmesh/int_range.h"

registerMooseObject("TensorMechanicsApp", ComputeMultipleInelasticStress);

InputParameters
ComputeMultipleInelasticStress::validParams()
{
  InputParameters params = ComputeMultipleInelasticStressBase::validParams();
  params.addRequiredParam<std::vector<MaterialName>>(
      "inelastic_models",
      "The material objects to use to calculate stress and inelastic strains. "
      "Note: specify creep models first and plasticity models second.");
  return params;
}

ComputeMultipleInelasticStress::ComputeMultipleInelasticStress(const InputParameters & parameters)
  : ComputeMultipleInelasticStressBase(parameters)
{
}

std::vector<MaterialName>
ComputeMultipleInelasticStress::getInelasticModelNames()
{
  return getParam<std::vector<MaterialName>>("inelastic_models");
}

void
ComputeMultipleInelasticStress::updateQpState(RankTwoTensor & elastic_strain_increment,
                                              RankTwoTensor & combined_inelastic_strain_increment)
{
  if (_internal_solve_full_iteration_history == true)
  {
    _console << std::endl
             << "iteration output for ComputeMultipleInelasticStress solve:"
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

      // initially assume the strain is completely elastic
      elastic_strain_increment = _strain_increment[_qp];
      // and subtract off all inelastic strain increments calculated so far
      // except the one that we're about to calculate
      for (const auto j_rmm : make_range(_num_models))
        if (i_rmm != j_rmm)
          elastic_strain_increment -= inelastic_strain_increment[j_rmm];

      // form the trial stress, with the check for changed elasticity constants
      if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
        _stress[_qp] =
            _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
      else
      {
        if (_damage_model)
          _stress[_qp] = _undamaged_stress_old + _elasticity_tensor[_qp] * elastic_strain_increment;
        else
          _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;
      }

      // given a trial stress (_stress[_qp]) and a strain increment (elastic_strain_increment)
      // let the i^th model produce an admissible stress (as _stress[_qp]), and decompose
      // the strain increment into an elastic part (elastic_strain_increment) and an
      // inelastic part (inelastic_strain_increment[i_rmm])
      computeAdmissibleState(i_rmm,
                             elastic_strain_increment,
                             inelastic_strain_increment[i_rmm],
                             _consistent_tangent_operator[i_rmm]);

      if (i_rmm == 0)
      {
        stress_max = _stress[_qp];
        stress_min = _stress[_qp];
      }
      else
      {
        for (const auto i : make_range(Moose::dim))
          for (const auto j : make_range(Moose::dim))
            if (_stress[_qp](i, j) > stress_max(i, j))
              stress_max(i, j) = _stress[_qp](i, j);
            else if (stress_min(i, j) > _stress[_qp](i, j))
              stress_min(i, j) = _stress[_qp](i, j);
      }
    }

    // now check convergence in the stress:
    // once the change in stress is within tolerance after each recompute material
    // consider the stress to be converged
    l2norm_delta_stress = (stress_max - stress_min).L2norm();
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
    throw MooseException("Max stress iteration hit during ComputeMultipleInelasticStress solve!");

  combined_inelastic_strain_increment.zero();
  for (const auto i_rmm : make_range(_num_models))
    combined_inelastic_strain_increment +=
        _inelastic_weights[i_rmm] * inelastic_strain_increment[i_rmm];

  if (_fe_problem.currentlyComputingJacobian())
    computeQpJacobianMult();

  _material_timestep_limit[_qp] = 0.0;
  for (const auto i_rmm : make_range(_num_models))
    _material_timestep_limit[_qp] += 1.0 / _models[i_rmm]->computeTimeStepLimit();

  if (MooseUtils::absoluteFuzzyEqual(_material_timestep_limit[_qp], 0.0))
    _material_timestep_limit[_qp] = std::numeric_limits<Real>::max();
  else
    _material_timestep_limit[_qp] = 1.0 / _material_timestep_limit[_qp];
}
