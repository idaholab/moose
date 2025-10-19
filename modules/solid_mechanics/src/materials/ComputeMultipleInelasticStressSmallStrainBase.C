//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMultipleInelasticStressSmallStrainBase.h"

#include "StressUpdateBase.h"
#include "MooseException.h"
#include "DamageBase.h"
#include "libmesh/int_range.h"

InputParameters
ComputeMultipleInelasticStressSmallStrainBase::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params.addClassDescription("Compute state (stress and internal parameters such as plastic "
                             "strains and internal parameters) using an iterative process with "
                             "small strain formulation. Combinations of creep models and plastic "
                             "models may be used.");
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
  MooseEnum tangent_operator("elastic nonlinear", "nonlinear");
  params.addParam<MooseEnum>(
      "tangent_operator",
      tangent_operator,
      "Type of tangent operator to return.  'elastic': return the "
      "elasticity tensor.  'nonlinear': return the full, general consistent tangent "
      "operator.");
  params.addParam<std::vector<Real>>("combined_inelastic_strain_weights",
                                     "The combined_inelastic_strain Material Property is a "
                                     "weighted sum of the model inelastic strains.  This parameter "
                                     "is a vector of weights, of the same length as "
                                     "inelastic_models.  Default = '1 1 ... 1'.  This "
                                     "parameter is set to 1 if the number of models = 1");
  params.addParam<bool>(
      "cycle_models", false, "At timestep N use only inelastic model N % num_models.");
  params.addParam<MaterialName>("damage_model", "Name of the damage model");

  return params;
}

ComputeMultipleInelasticStressSmallStrainBase::ComputeMultipleInelasticStressSmallStrainBase(
    const InputParameters & parameters)
  : ComputeStressBase(parameters),
    GuaranteeConsumer(this),
    _max_iterations(parameters.get<unsigned int>("max_iterations")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _internal_solve_full_iteration_history(getParam<bool>("internal_solve_full_iteration_history")),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _mechanical_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "mechanical_strain")),
    _inelastic_strain(declareProperty<RankTwoTensor>(_base_name + "combined_inelastic_strain")),
    _inelastic_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + "combined_inelastic_strain")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _tangent_operator_type(getParam<MooseEnum>("tangent_operator").getEnum<TangentOperatorEnum>()),
    _tangent_calculation_method(TangentCalculationMethod::ELASTIC),
    _cycle_models(getParam<bool>("cycle_models")),
    _material_timestep_limit(declareProperty<Real>(_base_name + "material_timestep_limit")),
    _identity_symmetric_four(RankFourTensor::initIdentitySymmetricFour),
    _all_models_isotropic(true)
{
}

void
ComputeMultipleInelasticStressSmallStrainBase::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();
  _inelastic_strain[_qp].zero();
}

void
ComputeMultipleInelasticStressSmallStrainBase::initialSetup()
{
  _damage_model = isParamValid("damage_model")
                      ? dynamic_cast<DamageBaseTempl<false> *>(&getMaterial("damage_model"))
                      : nullptr;

  _is_elasticity_tensor_guaranteed_isotropic =
      hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC);

  std::vector<MaterialName> models = getInelasticModelNames();

  _num_models = models.size();
  _tangent_computation_flag.resize(_num_models, false);
  _consistent_tangent_operator.resize(_num_models);

  _inelastic_weights = isParamValid("combined_inelastic_strain_weights")
                           ? getParam<std::vector<Real>>("combined_inelastic_strain_weights")
                           : std::vector<Real>(_num_models, 1.0);

  if (_inelastic_weights.size() != _num_models)
    mooseError("ComputeMultipleInelasticStressSmallStrainBase: combined_inelastic_strain_weights "
               "must contain the same number of entries as inelastic_models ",
               _inelastic_weights.size(),
               " ",
               _num_models);

  for (const auto i : make_range(_num_models))
  {
    StressUpdateBase * rrr = dynamic_cast<StressUpdateBase *>(&getMaterialByName(models[i]));

    if (rrr)
    {
      _models.push_back(rrr);
      _all_models_isotropic = _all_models_isotropic && rrr->isIsotropic();
      if (rrr->requiresIsotropicTensor() && !_is_elasticity_tensor_guaranteed_isotropic)
        mooseError("Model " + models[i] +
                   " requires an isotropic elasticity tensor, but the one supplied is not "
                   "guaranteed isotropic");
    }
    else
      mooseError("Model " + models[i] +
                 " is not compatible with ComputeMultipleInelasticStressSmallStrainBase");
  }

  // Check if tangent calculation methods are consistent
  if (_tangent_operator_type != TangentOperatorEnum::elastic)
  {
    bool full_present = false;
    bool partial_present = false;
    for (const auto i : make_range(_num_models))
    {
      if (_models[i]->getTangentCalculationMethod() == TangentCalculationMethod::FULL)
      {
        full_present = true;
        _tangent_computation_flag[i] = true;
        _tangent_calculation_method = TangentCalculationMethod::FULL;
      }
      else if (_models[i]->getTangentCalculationMethod() == TangentCalculationMethod::PARTIAL)
      {
        partial_present = true;
        _tangent_computation_flag[i] = true;
        _tangent_calculation_method = TangentCalculationMethod::PARTIAL;
      }
    }
    if (full_present && partial_present)
      mooseError("In ",
                 _name,
                 ": Models that calculate the full tangent operator and the partial tangent "
                 "operator are being combined. Either set tangent_operator to elastic, implement "
                 "the correct tangent formulations, or use different models.");
  }

  if (isParamValid("damage_model") && !_damage_model)
    paramError("damage_model",
               "Damage Model " + _damage_model->name() +
                   " is not compatible with ComputeMultipleInelasticStressSmallStrainBase");

  // Check for substepping capability
  for (const auto model_number : index_range(_models))
  {
    const bool use_substep = _models[model_number]->substeppingCapabilityRequested();
    if (use_substep && !_models[model_number]->substeppingCapabilityEnabled())
    {
      std::stringstream error_message;
      error_message << "Usage of substepping has been requested, but the inelastic model "
                    << _models[model_number]->name() << " does not implement substepping yet.";
      mooseError(error_message.str());
    }
  }
}

void
ComputeMultipleInelasticStressSmallStrainBase::computeQpStress()
{
  if (_damage_model)
  {
    _undamaged_stress_old = _stress_old[_qp];
    _damage_model->setQp(_qp);
    _damage_model->computeUndamagedOldStress(_undamaged_stress_old);
  }

  RankTwoTensor elastic_strain;
  RankTwoTensor inelastic_strain;

  if (_num_models == 0)
  {
    // No inelastic models: purely elastic response
    elastic_strain = _mechanical_strain[_qp];
    _elastic_strain[_qp] = elastic_strain;
    _stress[_qp] = _elasticity_tensor[_qp] * elastic_strain;

    if (_fe_problem.currentlyComputingJacobian())
      _Jacobian_mult[_qp] = _elasticity_tensor[_qp];

    _material_timestep_limit[_qp] = std::numeric_limits<Real>::max();
    inelastic_strain.zero();
  }
  else
  {
    if (_num_models == 1 || _cycle_models)
      updateQpStateSingleModel((_t_step - 1) % _num_models, elastic_strain, inelastic_strain);
    else
      updateQpState(elastic_strain, inelastic_strain);

    _elastic_strain[_qp] = elastic_strain;
  }

  _inelastic_strain[_qp] = inelastic_strain;

  if (_damage_model)
  {
    _damage_model->setQp(_qp);
    _damage_model->updateDamage();
    _damage_model->updateStressForDamage(_stress[_qp]);
    _damage_model->updateJacobianMultForDamage(_Jacobian_mult[_qp]);

    const Real damage_timestep_limit = _damage_model->computeTimeStepLimit();
    if (_material_timestep_limit[_qp] > damage_timestep_limit)
      _material_timestep_limit[_qp] = damage_timestep_limit;
  }
}

void
ComputeMultipleInelasticStressSmallStrainBase::computeQpJacobianMult()
{
  _Jacobian_mult[_qp] = MultipleInelasticStressHelper::computeJacobianMult(
      _tangent_calculation_method,
      _elasticity_tensor[_qp],
      _consistent_tangent_operator,
      _num_models,
      _identity_symmetric_four);
}

void
ComputeMultipleInelasticStressSmallStrainBase::updateQpStateSingleModel(
    unsigned model_number,
    RankTwoTensor & elastic_strain,
    RankTwoTensor & inelastic_strain)
{
  for (auto model : _models)
    model->setQp(_qp);

  // Compute trial stress assuming elastic response
  _stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];

  // Let the model compute admissible stress and strain decomposition
  computeAdmissibleState(
      model_number, elastic_strain, inelastic_strain, _consistent_tangent_operator[0]);

  if (_fe_problem.currentlyComputingJacobian())
  {
    if (_tangent_calculation_method == TangentCalculationMethod::ELASTIC)
      _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
    else if (_tangent_calculation_method == TangentCalculationMethod::PARTIAL)
    {
      RankFourTensor A = _identity_symmetric_four + _consistent_tangent_operator[0];
      mooseAssert(A.isSymmetric(), "Tangent operator isn't symmetric");
      _Jacobian_mult[_qp] = A.invSymm() * _elasticity_tensor[_qp];
    }
    else
      _Jacobian_mult[_qp] = _consistent_tangent_operator[0];
  }

  _material_timestep_limit[_qp] = _models[0]->computeTimeStepLimit();

  /* propagate internal variables, etc, to this timestep for those inelastic models where
   * "updateState" is not called */
  for (const auto i_rmm : make_range(_num_models))
    if (i_rmm != model_number)
      _models[i_rmm]->propagateQpStatefulProperties();
}

void
ComputeMultipleInelasticStressSmallStrainBase::computeAdmissibleState(
    unsigned model_number,
    RankTwoTensor & elastic_strain,
    RankTwoTensor & inelastic_strain,
    RankFourTensor & consistent_tangent_operator)
{
  // Reset properties to the beginning of the time step (necessary if substepping is employed).
  _models[model_number]->resetIncrementalMaterialProperties();

  // For small strain, we work with total strains
  // Compute strain increment for the stress update models
  RankTwoTensor strain_increment = _mechanical_strain[_qp] - _mechanical_strain_old[_qp];

  const bool jac = _fe_problem.currentlyComputingJacobian();

  // Rotation increment is identity for small strain
  RankTwoTensor rotation_increment;
  rotation_increment.setToIdentity();

  if (_damage_model)
    _models[model_number]->updateState(strain_increment,
                                       inelastic_strain,
                                       rotation_increment,
                                       _stress[_qp],
                                       _undamaged_stress_old,
                                       _elasticity_tensor[_qp],
                                       _elastic_strain[_qp],
                                       (jac && _tangent_computation_flag[model_number]),
                                       consistent_tangent_operator);
  else if (_models[model_number]->substeppingCapabilityEnabled() &&
           _is_elasticity_tensor_guaranteed_isotropic)
    _models[model_number]->updateStateSubstep(strain_increment,
                                              inelastic_strain,
                                              rotation_increment,
                                              _stress[_qp],
                                              _stress_old[_qp],
                                              _elasticity_tensor[_qp],
                                              _elastic_strain[_qp],
                                              (jac && _tangent_computation_flag[model_number]),
                                              consistent_tangent_operator);
  else
    _models[model_number]->updateState(strain_increment,
                                       inelastic_strain,
                                       rotation_increment,
                                       _stress[_qp],
                                       _stress_old[_qp],
                                       _elasticity_tensor[_qp],
                                       _elastic_strain[_qp],
                                       (jac && _tangent_computation_flag[model_number]),
                                       consistent_tangent_operator);

  // Compute elastic strain from mechanical strain and inelastic strain
  elastic_strain = _mechanical_strain[_qp] - _inelastic_strain_old[_qp] - inelastic_strain;

  if (jac && !_tangent_computation_flag[model_number])
  {
    if (_tangent_calculation_method == TangentCalculationMethod::PARTIAL)
      consistent_tangent_operator.zero();
    else
      consistent_tangent_operator = _elasticity_tensor[_qp];
  }
}
