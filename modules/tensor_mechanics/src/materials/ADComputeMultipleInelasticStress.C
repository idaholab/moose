//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeMultipleInelasticStress.h"
#include "MooseException.h"

registerMooseObject("TensorMechanicsApp", ADComputeMultipleInelasticStress);

InputParameters
ADComputeMultipleInelasticStress::validParams()
{
  InputParameters params = ADComputeFiniteStrainElasticStress::validParams();
  params.addClassDescription("Compute state (stress and internal parameters such as plastic "
                             "strains and internal parameters) using an iterative process.  "
                             "Combinations of creep models and plastic models may be used.");
  params.addParam<unsigned int>("max_iterations",
                                30,
                                "Maximum number of the stress update "
                                "iterations over the stress change after all "
                                "update materials are called");
  params.addParam<Real>("relative_tolerance",
                        1e-5,
                        "Relative convergence tolerance for the stress "
                        "update iterations over the stress change "
                        "after all update materials are called");
  params.addParam<Real>("absolute_tolerance",
                        1e-5,
                        "Absolute convergence tolerance for the stress "
                        "update iterations over the stress change "
                        "after all update materials are called");
  params.addParam<bool>(
      "internal_solve_full_iteration_history",
      false,
      "Set to true to output stress update iteration information over the stress change");
  params.addParam<bool>("perform_finite_strain_rotations",
                        true,
                        "Tensors are correctly rotated in "
                        "finite-strain simulations.  For "
                        "optimal performance you can set "
                        "this to 'false' if you are only "
                        "ever using small strains");
  params.addRequiredParam<std::vector<MaterialName>>(
      "inelastic_models",
      "The material objects to use to calculate stress and inelastic strains. "
      "Note: specify creep models first and plasticity models second.");
  params.addParam<std::vector<Real>>("combined_inelastic_strain_weights",
                                     "The combined_inelastic_strain Material Property is a "
                                     "weighted sum of the model inelastic strains.  This parameter "
                                     "is a vector of weights, of the same length as "
                                     "inelastic_models.  Default = '1 1 ... 1'.  This "
                                     "parameter is set to 1 if the number of models = 1");
  params.addParam<bool>(
      "cycle_models", false, "At time step N use only inelastic model N % num_models.");
  params.addParam<MaterialName>("damage_model", "Name of the damage model");

  return params;
}

ADComputeMultipleInelasticStress::ADComputeMultipleInelasticStress(
    const InputParameters & parameters)
  : ADComputeFiniteStrainElasticStress(parameters),
    _max_iterations(parameters.get<unsigned int>("max_iterations")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _internal_solve_full_iteration_history(getParam<bool>("internal_solve_full_iteration_history")),
    _perform_finite_strain_rotations(getParam<bool>("perform_finite_strain_rotations")),
    _inelastic_strain(declareADProperty<RankTwoTensor>(_base_name + "combined_inelastic_strain")),
    _inelastic_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + "combined_inelastic_strain")),
    _num_models(getParam<std::vector<MaterialName>>("inelastic_models").size()),
    _inelastic_weights(isParamValid("combined_inelastic_strain_weights")
                           ? getParam<std::vector<Real>>("combined_inelastic_strain_weights")
                           : std::vector<Real>(_num_models, true)),
    _cycle_models(getParam<bool>("cycle_models")),
    _material_timestep_limit(declareProperty<Real>(_base_name + "material_timestep_limit")),
    _is_elasticity_tensor_guaranteed_isotropic(false)
{
  if (_inelastic_weights.size() != _num_models)
    paramError("combined_inelastic_strain_weights",
               "must contain the same number of entries as inelastic_models ",
               _inelastic_weights.size(),
               " vs. ",
               _num_models);
}

void
ADComputeMultipleInelasticStress::initQpStatefulProperties()
{
  ADComputeFiniteStrainElasticStress::initQpStatefulProperties();
  _inelastic_strain[_qp].zero();
}

void
ADComputeMultipleInelasticStress::initialSetup()
{
  _damage_model = isParamValid("damage_model")
                      ? dynamic_cast<DamageBaseTempl<true> *>(&getMaterial("damage_model"))
                      : nullptr;

  if (isParamValid("damage_model") && !_damage_model)
    paramError("damage_model",
               "Damage Model " + getMaterial("damage_model").name() +
                   " is not compatible with ADComputeMultipleInelasticStress");

  _is_elasticity_tensor_guaranteed_isotropic =
      this->hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC);

  std::vector<MaterialName> models = getParam<std::vector<MaterialName>>("inelastic_models");

  for (unsigned int i = 0; i < _num_models; ++i)
  {
    ADStressUpdateBase * rrr =
        dynamic_cast<ADStressUpdateBase *>(&this->getMaterialByName(models[i]));

    if (rrr)
    {
      _models.push_back(rrr);
      if (rrr->requiresIsotropicTensor() && !_is_elasticity_tensor_guaranteed_isotropic)
        mooseError("Model " + models[i] +
                   " requires an isotropic elasticity tensor, but the one supplied is not "
                   "guaranteed isotropic");
    }
    else
      mooseError("Model " + models[i] + " is not compatible with ADComputeMultipleInelasticStress");
  }
}

void
ADComputeMultipleInelasticStress::computeQpStress()
{
  if (_damage_model)
  {
    _undamaged_stress_old = _stress_old[_qp];
    _damage_model->setQp(_qp);
    _damage_model->computeUndamagedOldStress(_undamaged_stress_old);
  }

  computeQpStressIntermediateConfiguration();

  if (_damage_model)
  {
    _damage_model->setQp(_qp);
    _damage_model->updateDamage();
    _damage_model->updateStressForDamage(_stress[_qp]);
    _damage_model->finiteStrainRotation(_rotation_increment[_qp]);

    const Real damage_timestep_limit = _damage_model->computeTimeStepLimit();
    if (_material_timestep_limit[_qp] > damage_timestep_limit)
      _material_timestep_limit[_qp] = damage_timestep_limit;
  }

  if (_perform_finite_strain_rotations)
    finiteStrainRotation();
}

void
ADComputeMultipleInelasticStress::computeQpStressIntermediateConfiguration()
{
  ADRankTwoTensor elastic_strain_increment;
  ADRankTwoTensor combined_inelastic_strain_increment;

  if (_num_models == 0)
  {
    _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp];

    // If the elasticity tensor values have changed and the tensor is isotropic,
    // use the old strain to calculate the old stress
    if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
      _stress[_qp] = _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + _strain_increment[_qp]);
    else
    {
      if (_damage_model)
        paramError(
            "damage_model",
            "Damage models cannot be used with inelastic models and elastic anisotropic behavior");

      ADRankFourTensor elasticity_tensor_rotated = _elasticity_tensor[_qp];
      elasticity_tensor_rotated.rotate(_rotation_total_old[_qp]);

      _stress[_qp] =
          elasticity_tensor_rotated * (_elastic_strain_old[_qp] + _strain_increment[_qp]);

      // Update current total rotation matrix to be used in next step
      _rotation_total[_qp] = _rotation_increment[_qp] * _rotation_total_old[_qp];
    }
  }
  else
  {
    if (_num_models == 1 || _cycle_models)
      updateQpStateSingleModel((_t_step - 1) % _num_models,
                               elastic_strain_increment,
                               combined_inelastic_strain_increment);
    else
      updateQpState(elastic_strain_increment, combined_inelastic_strain_increment);

    _elastic_strain[_qp] = _elastic_strain_old[_qp] + elastic_strain_increment;
    _inelastic_strain[_qp] = _inelastic_strain_old[_qp] + combined_inelastic_strain_increment;
  }
}

void
ADComputeMultipleInelasticStress::finiteStrainRotation()
{
  _elastic_strain[_qp] =
      _rotation_increment[_qp] * _elastic_strain[_qp] * _rotation_increment[_qp].transpose();
  _stress[_qp] = _rotation_increment[_qp] * _stress[_qp] * _rotation_increment[_qp].transpose();
  _inelastic_strain[_qp] =
      _rotation_increment[_qp] * _inelastic_strain[_qp] * _rotation_increment[_qp].transpose();
}

void
ADComputeMultipleInelasticStress::updateQpState(
    ADRankTwoTensor & elastic_strain_increment,
    ADRankTwoTensor & combined_inelastic_strain_increment)
{
  if (_internal_solve_full_iteration_history == true)
  {
    _console << std::endl
             << "iteration output for ADComputeMultipleInelasticStress solve:"
             << " time=" << _t << " int_pt=" << _qp << std::endl;
  }
  Real l2norm_delta_stress;
  Real first_l2norm_delta_stress = 1.0;
  unsigned int counter = 0;

  std::vector<ADRankTwoTensor> inelastic_strain_increment;
  inelastic_strain_increment.resize(_num_models);

  for (unsigned i_rmm = 0; i_rmm < _models.size(); ++i_rmm)
    inelastic_strain_increment[i_rmm].zero();

  ADRankTwoTensor stress_max, stress_min;

  do
  {
    for (unsigned i_rmm = 0; i_rmm < _num_models; ++i_rmm)
    {
      _models[i_rmm]->setQp(_qp);

      // initially assume the strain is completely elastic
      elastic_strain_increment = _strain_increment[_qp];
      // and subtract off all inelastic strain increments calculated so far
      // except the one that we're about to calculate
      for (unsigned j_rmm = 0; j_rmm < _num_models; ++j_rmm)
        if (i_rmm != j_rmm)
          elastic_strain_increment -= inelastic_strain_increment[j_rmm];

      // form the trial stress, with the check for changed elasticity constants
      if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
        _stress[_qp] =
            _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
      else
      {
        if (_damage_model)
          paramError("damage_model",
                     "Damage models cannot be used with inelastic models and elastic anisotropic "
                     "behavior");

        ADRankFourTensor elasticity_tensor_rotated = _elasticity_tensor[_qp];
        elasticity_tensor_rotated.rotate(_rotation_total_old[_qp]);

        _stress[_qp] =
            elasticity_tensor_rotated * (_elastic_strain_old[_qp] + elastic_strain_increment);

        // Update current total rotation matrix to be used in next step
        _rotation_total[_qp] = _rotation_increment[_qp] * _rotation_total_old[_qp];
      }
      // given a trial stress (_stress[_qp]) and a strain increment (elastic_strain_increment)
      // let the i^th model produce an admissible stress (as _stress[_qp]), and decompose
      // the strain increment into an elastic part (elastic_strain_increment) and an
      // inelastic part (inelastic_strain_increment[i_rmm])
      computeAdmissibleState(i_rmm, elastic_strain_increment, inelastic_strain_increment[i_rmm]);

      if (i_rmm == 0)
      {
        stress_max = _stress[_qp];
        stress_min = _stress[_qp];
      }
      else
      {
        for (const auto i : make_range(Moose::dim))
        {
          for (const auto j : make_range(Moose::dim))
          {
            if (_stress[_qp](i, j) > stress_max(i, j))
              stress_max(i, j) = _stress[_qp](i, j);
            else if (stress_min(i, j) > _stress[_qp](i, j))
              stress_min(i, j) = _stress[_qp](i, j);
          }
        }
      }
    }

    // now check convergence in the stress:
    // once the change in stress is within tolerance after each recompute material
    // consider the stress to be converged
    l2norm_delta_stress = MetaPhysicL::raw_value((stress_max - stress_min).L2norm());
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
    mooseException(
        "In ", _name, ": Max stress iteration hit during ADComputeMultipleInelasticStress solve!");

  combined_inelastic_strain_increment.zero();
  for (unsigned i_rmm = 0; i_rmm < _num_models; ++i_rmm)
    combined_inelastic_strain_increment +=
        _inelastic_weights[i_rmm] * inelastic_strain_increment[i_rmm];

  _material_timestep_limit[_qp] = 0.0;
  for (unsigned i_rmm = 0; i_rmm < _num_models; ++i_rmm)
    _material_timestep_limit[_qp] += 1.0 / _models[i_rmm]->computeTimeStepLimit();

  if (MooseUtils::absoluteFuzzyEqual(_material_timestep_limit[_qp], 0.0))
    _material_timestep_limit[_qp] = std::numeric_limits<Real>::max();
  else
    _material_timestep_limit[_qp] = 1.0 / _material_timestep_limit[_qp];
}

void
ADComputeMultipleInelasticStress::updateQpStateSingleModel(
    unsigned model_number,
    ADRankTwoTensor & elastic_strain_increment,
    ADRankTwoTensor & combined_inelastic_strain_increment)
{
  for (auto model : _models)
    model->setQp(_qp);

  elastic_strain_increment = _strain_increment[_qp];

  // If the elasticity tensor values have changed and the tensor is isotropic,
  // use the old strain to calculate the old stress
  if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
    _stress[_qp] = _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
  else
  {
    if (_damage_model)
      paramError(
          "damage_model",
          "Damage models cannot be used with inelastic models and elastic anisotropic behavior");

    ADRankFourTensor elasticity_tensor_rotated = _elasticity_tensor[_qp];
    elasticity_tensor_rotated.rotate(_rotation_total_old[_qp]);

    _stress[_qp] =
        elasticity_tensor_rotated * (_elastic_strain_old[_qp] + elastic_strain_increment);

    // Update current total rotation matrix to be used in next step
    _rotation_total[_qp] = _rotation_increment[_qp] * _rotation_total_old[_qp];
  }

  computeAdmissibleState(
      model_number, elastic_strain_increment, combined_inelastic_strain_increment);

  _material_timestep_limit[_qp] = _models[0]->computeTimeStepLimit();

  /* propagate internal variables, etc, to this timestep for those inelastic models where
   * "updateState" is not called */
  for (unsigned i_rmm = 0; i_rmm < _num_models; ++i_rmm)
    if (i_rmm != model_number)
      _models[i_rmm]->propagateQpStatefulProperties();
}

void
ADComputeMultipleInelasticStress::computeAdmissibleState(
    unsigned model_number,
    ADRankTwoTensor & elastic_strain_increment,
    ADRankTwoTensor & inelastic_strain_increment)
{
  // Properly update material properties (necessary if substepping is employed).
  _models[model_number]->resetIncrementalMaterialProperties();

  if (_damage_model)
    _models[model_number]->updateState(elastic_strain_increment,
                                       inelastic_strain_increment,
                                       _rotation_increment[_qp],
                                       _stress[_qp],
                                       _undamaged_stress_old,
                                       _elasticity_tensor[_qp],
                                       _elastic_strain_old[_qp]);
  else if (_models[model_number]->substeppingCapabilityEnabled() &&
           (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations))
  {
    _models[model_number]->updateStateSubstep(elastic_strain_increment,
                                              inelastic_strain_increment,
                                              _rotation_increment[_qp],
                                              _stress[_qp],
                                              _stress_old[_qp],
                                              _elasticity_tensor[_qp],
                                              _elastic_strain_old[_qp]);
  }
  else
    _models[model_number]->updateState(elastic_strain_increment,
                                       inelastic_strain_increment,
                                       _rotation_increment[_qp],
                                       _stress[_qp],
                                       _stress_old[_qp],
                                       _elasticity_tensor[_qp],
                                       _elastic_strain_old[_qp]);
}
