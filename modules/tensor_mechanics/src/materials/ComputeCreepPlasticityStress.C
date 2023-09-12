//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCreepPlasticityStress.h"

#include "ElasticityTensorTools.h"

#include "MooseException.h"
#include "libmesh/int_range.h"

registerMooseObject("TensorMechanicsApp", ComputeCreepPlasticityStress);

InputParameters
ComputeCreepPlasticityStress::validParams()
{
  InputParameters params = ComputeMultipleInelasticStressBase::validParams();
  params.addClassDescription("Compute state (stress and internal parameters such as inelastic "
                             "strains and internal parameters) using an Newton process for one "
                             "creep and one plasticity model");
  params.addRequiredParam<MaterialName>("creep_model",
                                        "Creep model that derives from PowerLawCreepStressUpdate.");
  params.addRequiredParam<MaterialName>(
      "plasticity_model", "Plasticity model that derives from IsotropicPlasticityStressUpdate.");

  return params;
}

ComputeCreepPlasticityStress::ComputeCreepPlasticityStress(const InputParameters & parameters)
  : ComputeMultipleInelasticStressBase(parameters)
{
}

std::vector<MaterialName>
ComputeCreepPlasticityStress::getInelasticModelNames()
{
  std::vector<MaterialName> names = {getParam<MaterialName>("creep_model"),
                                     getParam<MaterialName>("plasticity_model")};
  return names;
}

void
ComputeCreepPlasticityStress::initialSetup()
{
  ComputeMultipleInelasticStressBase::initialSetup();

  if (_models.size() != 2)
    mooseError("Error in ComputeCreepPlasticityStress: two models are required.");

  _creep_model = dynamic_cast<PowerLawCreepStressUpdate *>(_models[0]);
  if (!_creep_model)
    mooseError("Model " + getParam<MaterialName>("creep_model") +
               " is not a compatible creep model in ComputeCreepPlasticityStress.");

  _plasticity_model = dynamic_cast<IsotropicPlasticityStressUpdate *>(_models[1]);
  if (!_plasticity_model)
    mooseError("Model " + getParam<MaterialName>("plasticity_model") +
               " is not a compatible plasticity model in ComputeCreepPlasticityStress.");
}

void
ComputeCreepPlasticityStress::updateQpState(RankTwoTensor & elastic_strain_increment,
                                            RankTwoTensor & combined_inelastic_strain_increment)
{
  if (_internal_solve_full_iteration_history == true)
  {
    _console << std::endl
             << "iteration output for ComputeCreepPlasticityStress solve:"
             << " time=" << _t << " int_pt=" << _qp << std::endl;
  }

  for (const auto i_rmm : index_range(_models))
    _inelastic_strain_increment[i_rmm].zero();

  elastic_strain_increment = _strain_increment[_qp];
  computeStress(elastic_strain_increment, _stress[_qp]);

  const RankTwoTensor trial_stress = _stress[_qp];
  const RankTwoTensor deviatoric_trial_stress = trial_stress.deviatoric();
  const Real effective_trial_stress =
      std::sqrt(1.5 * deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress));

  //
  // First, compute creep response assuming no plasticity
  //
  _creep_model->setQp(_qp);
  computeAdmissibleState(
      0, elastic_strain_increment, _inelastic_strain_increment[0], _consistent_tangent_operator[0]);
  _effective_creep_strain_increment = _creep_model->effectiveInelasticStrainIncrement();

  //
  // Now check the plasticity model.
  // If no yielding, we are done.
  //
  _plasticity_model->setQp(_qp);
  computeAdmissibleState(
      1, elastic_strain_increment, _inelastic_strain_increment[1], _consistent_tangent_operator[1]);
  _effective_plastic_strain_increment = _plasticity_model->effectiveInelasticStrainIncrement();
  const Real yield_condition = _plasticity_model->yieldCondition();

  if (yield_condition > 0.0) // yielding even with maximum creep strain
  {
    const Real max_effective_creep_strain_increment = _effective_creep_strain_increment;

    //
    // Compute plastic response given no creep
    //
    elastic_strain_increment = _strain_increment[_qp];
    _stress[_qp] = trial_stress;
    computeAdmissibleState(1,
                           elastic_strain_increment,
                           _inelastic_strain_increment[1],
                           _consistent_tangent_operator[1]);
    const Real max_effective_plastic_strain_increment =
        _plasticity_model->effectiveInelasticStrainIncrement();

    //
    // Reset plasticity model
    //
    _plasticity_model->computeStressInitialize(effective_trial_stress, _elasticity_tensor[_qp]);

    //
    // The residual for the creep law is (creep rate)*dt - (creep strain increment)
    // We want the creep rate calculation to depend on both creep and plasticity.
    // Since we send in the total scalar inelastic strain (creep and plasticity), we need to correct
    // the second term by adding the plastic strain.
    //
    Real creep_residual = _creep_model->computeResidual(effective_trial_stress,
                                                        _effective_creep_strain_increment +
                                                            _effective_plastic_strain_increment);
    creep_residual += _effective_plastic_strain_increment;

    //
    // We want the plasticity resdiual to be (effective_trial_stress - r - yield_stress)/3G - (total
    // inelastic increment)
    // The standard residual for plasticity is (effective_trial_stress - r - yield_stress)/3G -
    // (plasticity strain increment)
    // Since we send in the plastic inelastic strain (for calculation of r), we must subtract the
    // creep strain to correct the final term in our desired residual
    //
    Real plasticity_residual = _plasticity_model->computeResidual(
        effective_trial_stress, _effective_plastic_strain_increment);
    plasticity_residual -= _effective_creep_strain_increment;

    unsigned int counter = 0;
    Real residual = 0;
    const Real threeG =
        3.0 * ElasticityTensorTools::getIsotropicShearModulus(_elasticity_tensor[_qp]);
    const Real reference_residual = effective_trial_stress / threeG;

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
                                                 _effective_creep_strain_increment +
                                                     _effective_plastic_strain_increment);
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
      Real A22 = _plasticity_model->computeDerivative(effective_trial_stress,
                                                      _effective_plastic_strain_increment);

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

      Real x2 = rhs2 / A22;
      Real x1 = (rhs1 - A12 * x2) / A11;

      while (_effective_creep_strain_increment - x1 < 0)
        x1 *= 0.5;
      while (_effective_plastic_strain_increment - x2 < 0)
        x2 *= 0.5;

      while (_effective_creep_strain_increment - x1 > max_effective_creep_strain_increment)
        x1 *= 0.5;
      while (_effective_plastic_strain_increment - x2 > max_effective_plastic_strain_increment)
        x2 *= 0.5;

      // This check is to avoid a fpe in the creep law.
      // Maybe it is better to check for this condition in the creep law
      if (effective_trial_stress < threeG * (_effective_creep_strain_increment - x1 +
                                             _effective_plastic_strain_increment - x2))
      {
        const Real sum =
            _effective_creep_strain_increment - x1 + _effective_plastic_strain_increment - x2;
        const Real factor = 0.9 * effective_trial_stress / (threeG * sum);
        const Real cNew = (_effective_creep_strain_increment - x1) * factor;
        const Real pNew = (_effective_plastic_strain_increment - x2) * factor;
        x1 = _effective_creep_strain_increment - cNew;
        x2 = _effective_plastic_strain_increment - pNew;
      }

      _effective_creep_strain_increment -= x1;
      _effective_plastic_strain_increment -= x2;

      //
      // The residual for the creep law is (creep rate)*dt - (creep strain increment)
      //
      creep_residual = _creep_model->computeResidual(effective_trial_stress,
                                                     _effective_creep_strain_increment +
                                                         _effective_plastic_strain_increment);
      creep_residual += _effective_plastic_strain_increment;
      //
      // The residual for plasticity is (effective_trial_stress - r - yield_stress)/3G - scalar
      //
      plasticity_residual = _plasticity_model->computeResidual(effective_trial_stress,
                                                               _effective_plastic_strain_increment);
      plasticity_residual -= _effective_creep_strain_increment;

      residual =
          std::sqrt(creep_residual * creep_residual + plasticity_residual * plasticity_residual);

      if (_internal_solve_full_iteration_history == true)
      {
        _console << "stress iteration number = " << counter << "\n relative residual = "
                 << (0 == reference_residual ? 0 : residual / reference_residual)
                 << "\n stress convergence relative tolerance = " << _relative_tolerance
                 << "\n absolute residual = " << residual
                 << "\n creep residual = " << creep_residual
                 << "\n plasticity residual = " << plasticity_residual
                 << "\n creep iteration increment = " << x1
                 << "\n plasticity iteration increment = " << x2
                 << "\n creep scalar strain = " << _effective_creep_strain_increment
                 << "\n plasticity scalar strain = " << _effective_plastic_strain_increment
                 << "\n max creep scalar strain = " << max_effective_creep_strain_increment
                 << "\n max plasticity scalar strain = " << max_effective_plastic_strain_increment
                 << "\n stress convergence absolute tolerance = " << _absolute_tolerance
                 << std::endl;
      }
      ++counter;
    } while (counter < _max_iterations && residual > _absolute_tolerance &&
             (residual / reference_residual) > _relative_tolerance);

    if (counter == _max_iterations && residual > _absolute_tolerance &&
        (residual / reference_residual) > _relative_tolerance)
    {
      std::stringstream msg;
      msg << "\n relative residual = "
          << (0 == reference_residual ? 0 : residual / reference_residual)
          << "\n stress convergence relative tolerance = " << _relative_tolerance
          << "\n absolute residual = " << residual << "\n creep residual = " << creep_residual
          << "\n plasticity residual = " << plasticity_residual
          << "\n creep scalar strain = " << _effective_creep_strain_increment
          << "\n plasticity scalar strain = " << _effective_plastic_strain_increment
          << "\n max creep scalar strain = " << max_effective_creep_strain_increment
          << "\n max plasticity scalar strain = " << max_effective_plastic_strain_increment
          << "\n stress convergence absolute tolerance = " << _absolute_tolerance << std::endl;

      throw MooseException("Max stress iteration hit during ComputeCreepPlasticityStress solve! " +
                           _name + msg.str());
    }
  }
  computeInelasticStrainIncrements(effective_trial_stress, deviatoric_trial_stress);

  finalizeConstitutiveModels();

  combined_inelastic_strain_increment.zero();
  for (const auto i_rmm : make_range(_num_models))
    combined_inelastic_strain_increment += _inelastic_strain_increment[i_rmm];

  if (yield_condition > 0.0)
  {
    elastic_strain_increment = _strain_increment[_qp] - combined_inelastic_strain_increment;
    _stress[_qp] = _elasticity_tensor[_qp] * (elastic_strain_increment + _elastic_strain_old[_qp]);

    computeTangentOperators();
  }
  else
  {
    // The tangent for creep was called in the initial creep solve.

    // Set the tangent for plasticity to zero
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

void
ComputeCreepPlasticityStress::computeStress(const RankTwoTensor & elastic_strain_increment,
                                            RankTwoTensor & stress)
{
  // form the stress, with the check for changed elasticity constants
  if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
    stress = _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
  else
  {
    if (_damage_model)
      stress = _undamaged_stress_old + _elasticity_tensor[_qp] * elastic_strain_increment;
    else
      stress = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;
  }
}

void
ComputeCreepPlasticityStress::computeTangentOperators()
{
  auto elastic_strain_inc = _strain_increment[_qp] - _inelastic_strain_increment[1];

  RankTwoTensor stress;
  computeStress(elastic_strain_inc, stress);
  RankTwoTensor deviatoric_stress = stress.deviatoric();
  Real effective_stress = std::sqrt(1.5 * deviatoric_stress.doubleContraction(deviatoric_stress));

  _creep_model->computeTangentOperator(
      effective_stress, _stress[_qp], _consistent_tangent_operator[0]);

  elastic_strain_inc = _strain_increment[_qp] - _inelastic_strain_increment[0];

  computeStress(elastic_strain_inc, stress);
  deviatoric_stress = stress.deviatoric();
  effective_stress = std::sqrt(1.5 * deviatoric_stress.doubleContraction(deviatoric_stress));

  _plasticity_model->computeTangentOperator(
      effective_stress, _stress[_qp], _consistent_tangent_operator[1]);
}

void
ComputeCreepPlasticityStress::computeInelasticStrainIncrements(
    Real effective_trial_stress, const RankTwoTensor & deviatoric_trial_stress)
{
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    if (_effective_creep_strain_increment != 0.0)
      _inelastic_strain_increment[0] =
          deviatoric_trial_stress *
          (1.5 * _effective_creep_strain_increment / effective_trial_stress);
    else
      _inelastic_strain_increment[0].zero();

    if (_effective_plastic_strain_increment != 0.0)
      _inelastic_strain_increment[1] =
          deviatoric_trial_stress *
          (1.5 * _effective_plastic_strain_increment / effective_trial_stress);
    else
      _inelastic_strain_increment[1].zero();
  }
  else
  {
    _inelastic_strain_increment[0].zero();
    _inelastic_strain_increment[1].zero();
  }
}

void
ComputeCreepPlasticityStress::finalizeConstitutiveModels()
{
  _creep_model->updateEffectiveInelasticStrainIncrement(_effective_creep_strain_increment);
  _plasticity_model->updateEffectiveInelasticStrainIncrement(_effective_plastic_strain_increment);
  _creep_model->updateEffectiveInelasticStrain(_effective_creep_strain_increment);
  _plasticity_model->updateEffectiveInelasticStrain(_effective_plastic_strain_increment);
  _creep_model->resetIncrementalMaterialProperties();
  _creep_model->computeStressFinalize(_inelastic_strain_increment[0]);
  _plasticity_model->computeStressFinalize(_inelastic_strain_increment[1]);
}
