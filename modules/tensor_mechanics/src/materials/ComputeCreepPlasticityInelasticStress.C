//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCreepPlasticityInelasticStress.h"

#include "ElasticityTensorTools.h"

#include "MooseException.h"
#include "libmesh/int_range.h"

registerMooseObject("TensorMechanicsApp", ComputeCreepPlasticityInelasticStress);

InputParameters
ComputeCreepPlasticityInelasticStress::validParams()
{
  InputParameters params = ComputeMultipleInelasticStressBase::validParams();
  params.addClassDescription("Compute state (stress and internal parameters such as inelastic "
                             "strains and internal parameters) using an Newton process for one "
                             "creep and one plasticity model, given in that order.");
  return params;
}

ComputeCreepPlasticityInelasticStress::ComputeCreepPlasticityInelasticStress(
    const InputParameters & parameters)
  : ComputeMultipleInelasticStressBase(parameters)
{
  if (_num_models != 2)
    mooseError("ComputeCreepPlasticityInelasticStress requires exactly two inelastic models.");
}

void
ComputeCreepPlasticityInelasticStress::initialSetup()
{
  ComputeMultipleInelasticStressBase::initialSetup();

  const std::vector<MaterialName> & models =
      getParam<std::vector<MaterialName>>("inelastic_models");

  _creep_model = dynamic_cast<PowerLawCreepStressUpdate *>(&getMaterialByName(models[0]));
  if (!_creep_model)
    mooseError("Model " + models[0] +
               " is not a compatible creep model in ComputeCreepPlasticityInelasticStress.");
  _plasticity_model =
      dynamic_cast<IsotropicPlasticityStressUpdate *>(&getMaterialByName(models[1]));
  if (!_plasticity_model)
    mooseError("Model " + models[1] +
               " is not a compatible plasticity model in ComputeCreepPlasticityInelasticStress.");
}

void
ComputeCreepPlasticityInelasticStress::updateQpState(
    RankTwoTensor & elastic_strain_increment, RankTwoTensor & combined_inelastic_strain_increment)
{
  if (_internal_solve_full_iteration_history == true)
  {
    _console << std::endl
             << "iteration output for ComputeCreepPlasticityInelasticStress solve:"
             << " time=" << _t << " int_pt=" << _qp << std::endl;
  }

  std::vector<RankTwoTensor> inelastic_strain_increment(2);

  for (const auto i_rmm : index_range(_models))
    inelastic_strain_increment[i_rmm].zero();

  //
  // First, compute creep response assuming no plasticity
  //
  _creep_model->setQp(_qp);
  elastic_strain_increment = _strain_increment[_qp];
  // form the trial stress, with the check for changed elasticity constants
  if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
    _stress[_qp] = _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
  else
  {
    _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;
  }
  const RankTwoTensor deviatoric_trial_stress = _stress[_qp].deviatoric();
  const Real effective_trial_stress =
      std::sqrt(1.5 * deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress));

  updateQpStateSingleModel(0, elastic_strain_increment, inelastic_strain_increment[0]);
  Real scalar_creep_strain = _creep_model->scalarEffectiveInelasticStrain();

  //
  // Now check the plasticity model.
  // If no yielding, we are done.
  //
  _plasticity_model->setQp(_qp);
  Real scalar_plasticity_strain = 0;
  {
    const RankTwoTensor deviatoric_trial_stress = _stress[_qp].deviatoric();
    const Real effective_trial_stress =
        std::sqrt(1.5 * deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress));
    _plasticity_model->computeStressInitialize(effective_trial_stress, _elasticity_tensor[_qp]);
  }
  const Real yield_condition = _plasticity_model->yieldCondition();

  if (yield_condition > 0.0) // yielding
  {
    const Real threeG =
        3.0 * ElasticityTensorTools::getIsotropicShearModulus(_elasticity_tensor[_qp]);
    unsigned int counter = 0;
    Real residual = 0;
    Real reference_residual = 0;

    //
    // The residual for the creep law is (creep rate)*dt - (creep strain increment)
    // We want the creep rate calculation to depend on both creep and plasticity.
    // Since we send in the total scalar inelastic strain (creep and plasticity), we need to correct
    // the second term by adding the plastic strain.
    //
    Real creep_residual = _creep_model->computeResidual(
        effective_trial_stress, scalar_creep_strain + scalar_plasticity_strain);
    creep_residual += scalar_plasticity_strain;

    //
    // We want the resdiual to be (effective_trial_stress - r - yield_stress)/3G - (total inelastic
    // increment)
    // The standard residual for plasticity is (effective_trial_stress - r - yield_stress)/3G -
    // (plasticity strain increment)
    // Since we send in the plastic inelastic strain (for calculation of r), we must subtract the
    // creep strain to correct the final term in our desired residual
    //
    Real plasticity_residual =
        _plasticity_model->computeResidual(effective_trial_stress, scalar_plasticity_strain);
    plasticity_residual -= scalar_creep_strain;

    do
    {
      //
      // Solve Ax=b where x is the vector of creep and plastic inelastic strains
      //
      // x1 => creep
      // x2 => plasticity
      //
      //
      // A11 (creep,creep)
      //
      Real A11 = _creep_model->computeDerivative(effective_trial_stress,
                                                 scalar_creep_strain + scalar_plasticity_strain);
      //
      // A12 (creep,plasticity)
      //
      Real A12 = A11 + 1.0;
      //
      // A21 (plasticity,creep)
      //
      Real A21 = -1.0;
      //
      // A22 (plasticity,plasticity)
      //
      Real A22 =
          _plasticity_model->computeDerivative(effective_trial_stress, scalar_plasticity_strain);

      Real rhs1 = creep_residual;
      Real rhs2 = plasticity_residual;

      //
      // Solve
      //
      // A11*a + A21 = 0
      // A11*a = -A21
      // a = -A21/A11
      const Real a = -A21 / A11;
      A21 = 0;
      A22 += a * A12;
      rhs2 += a * rhs1;

      const Real x2 = rhs2 / A22;
      const Real x1 = (rhs1 - A12 * x2) / A11;

      scalar_creep_strain -= x1;
      scalar_plasticity_strain -= x2;

      //
      // The residual for the creep law is (creep rate)*dt - (creep strain increment)
      //
      creep_residual = _creep_model->computeResidual(
          effective_trial_stress, scalar_creep_strain + scalar_plasticity_strain);
      creep_residual += scalar_plasticity_strain;
      //
      // The residual for plasticity is (effective_trial_stress - r - yield_stress)/3G - scalar
      //
      plasticity_residual =
          _plasticity_model->computeResidual(effective_trial_stress, scalar_plasticity_strain);
      plasticity_residual -= scalar_creep_strain;

      residual =
          std::sqrt(creep_residual * creep_residual + plasticity_residual * plasticity_residual);
      reference_residual = effective_trial_stress / threeG;

      if (_internal_solve_full_iteration_history == true)
      {
        _console << "stress iteration number = " << counter << "\n"
                 << " relative residual = "
                 << (0 == reference_residual ? 0 : residual / reference_residual) << "\n"
                 << " stress convergence relative tolerance = " << _relative_tolerance << "\n"
                 << " absolute residual = " << residual << "\n"
                 << " creep residual = " << creep_residual << "\n"
                 << " plasticity residual = " << plasticity_residual << "\n"
                 << " creep iteration increment = " << x1 << "\n"
                 << " plasticity iteration increment = " << x2 << "\n"
                 << " creep scalar strain = " << scalar_creep_strain << "\n"
                 << " plasticity scalar strain = " << scalar_plasticity_strain << "\n"
                 << " stress convergence absolute tolerance = " << _absolute_tolerance << std::endl;
      }
      ++counter;
    } while (counter < _max_iterations && residual > _absolute_tolerance &&
             (residual / reference_residual) > _relative_tolerance);

    if (counter == _max_iterations && residual > _absolute_tolerance &&
        (residual / reference_residual) > _relative_tolerance)
      throw MooseException(
          "Max stress iteration hit during ComputeCreepPlasticityInelasticStress solve!");
  }
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    if (scalar_creep_strain != 0.0)
      inelastic_strain_increment[0] =
          deviatoric_trial_stress * (1.5 * scalar_creep_strain / effective_trial_stress);
    else
      inelastic_strain_increment[0].zero();

    if (scalar_plasticity_strain != 0.0)
      inelastic_strain_increment[1] =
          deviatoric_trial_stress * (1.5 * scalar_plasticity_strain / effective_trial_stress);
    else
      inelastic_strain_increment[1].zero();
  }
  else
  {
    inelastic_strain_increment[0].zero();
    inelastic_strain_increment[1].zero();
  }

  _creep_model->updateScalarEffectiveInelasticStrain(scalar_creep_strain);
  _plasticity_model->updateScalarEffectiveInelasticStrain(scalar_plasticity_strain);
  _creep_model->updateEffectiveInelasticStrain(scalar_creep_strain);
  _plasticity_model->updateEffectiveInelasticStrain(scalar_plasticity_strain);
  _creep_model->resetIncrementalMaterialProperties();
  _creep_model->computeStressFinalize(inelastic_strain_increment[0]);
  _plasticity_model->computeStressFinalize(inelastic_strain_increment[1]);

  combined_inelastic_strain_increment.zero();
  for (const auto i_rmm : make_range(_num_models))
    combined_inelastic_strain_increment += inelastic_strain_increment[i_rmm];

  if (yield_condition > 0.0)
  {
    elastic_strain_increment = _strain_increment[_qp] - combined_inelastic_strain_increment;
    _stress[_qp] = _elasticity_tensor[_qp] * (elastic_strain_increment + _elastic_strain_old[_qp]);

    auto elastic_strain_increment = _strain_increment[_qp] - inelastic_strain_increment[1];

    // form the trial stress, with the check for changed elasticity constants
    RankTwoTensor stress;
    if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
      stress = _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
    else
    {
      if (_damage_model)
        stress = _undamaged_stress_old + _elasticity_tensor[_qp] * elastic_strain_increment;
      else
        stress = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;
    }

    RankTwoTensor deviatoric_trial_stress = stress.deviatoric();
    Real effective_trial_stress =
        std::sqrt(1.5 * deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress));

    _creep_model->computeTangentOperator(
        effective_trial_stress, _stress[_qp], _consistent_tangent_operator[0]);

    elastic_strain_increment = _strain_increment[_qp] - inelastic_strain_increment[0];

    // form the trial stress, with the check for changed elasticity constants
    if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
      stress = _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
    else
    {
      if (_damage_model)
        stress = _undamaged_stress_old + _elasticity_tensor[_qp] * elastic_strain_increment;
      else
        stress = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;
    }

    deviatoric_trial_stress = stress.deviatoric();
    effective_trial_stress =
        std::sqrt(1.5 * deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress));

    _plasticity_model->computeTangentOperator(
        effective_trial_stress, _stress[_qp], _consistent_tangent_operator[1]);
  }
  else
  {
    _consistent_tangent_operator[1].zero();
  }

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
