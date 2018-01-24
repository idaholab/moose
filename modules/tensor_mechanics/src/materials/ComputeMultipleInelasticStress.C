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

template <>
InputParameters
validParams<ComputeMultipleInelasticStress>()
{
  InputParameters params = validParams<ComputeFiniteStrainElasticStress>();
  params.addClassDescription("Compute state (stress and internal parameters such as plastic "
                             "strains and internal parameters) using an iterative process.  "
                             "Combinations of creep models and plastic models may be used");
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
      "output_iteration_info",
      false,
      "Set to true to output stress update iteration information over the stress change");
  params.addParam<bool>("perform_finite_strain_rotations",
                        true,
                        "Tensors are correctly rotated in "
                        "finite-strain simulations.  For "
                        "optimal performance you can set "
                        "this to 'false' if you are only "
                        "ever using small strains");
  MooseEnum tangent_operator("elastic nonlinear", "nonlinear");
  params.addParam<MooseEnum>(
      "tangent_operator",
      tangent_operator,
      "Type of tangent operator to return.  'elastic': return the "
      "elasticity tensor.  'nonlinear': return the full, general consistent tangent "
      "operator.");
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
      "cycle_models", false, "At timestep N use only inelastic model N % num_models.");
  return params;
}

ComputeMultipleInelasticStress::ComputeMultipleInelasticStress(const InputParameters & parameters)
  : ComputeFiniteStrainElasticStress(parameters),
    _max_iterations(parameters.get<unsigned int>("max_iterations")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _perform_finite_strain_rotations(getParam<bool>("perform_finite_strain_rotations")),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),
    _elastic_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "elastic_strain")),
    _strain_increment(getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _inelastic_strain(declareProperty<RankTwoTensor>(_base_name + "combined_inelastic_strain")),
    _inelastic_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + "combined_inelastic_strain")),
    _tangent_operator_type(getParam<MooseEnum>("tangent_operator").getEnum<TangentOperatorEnum>()),
    _num_models(getParam<std::vector<MaterialName>>("inelastic_models").size()),
    _inelastic_weights(isParamValid("combined_inelastic_strain_weights")
                           ? getParam<std::vector<Real>>("combined_inelastic_strain_weights")
                           : std::vector<Real>(_num_models, true)),
    _consistent_tangent_operator(_num_models),
    _cycle_models(getParam<bool>("cycle_models")),
    _matl_timestep_limit(declareProperty<Real>("matl_timestep_limit"))
{
  if (_inelastic_weights.size() != _num_models)
    mooseError(
        "ComputeMultipleInelasticStress: combined_inelastic_strain_weights must contain the same "
        "number of entries as inelastic_models ",
        _inelastic_weights.size(),
        " ",
        _num_models);
}

void
ComputeMultipleInelasticStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();
  _inelastic_strain[_qp].zero();
}

void
ComputeMultipleInelasticStress::initialSetup()
{
  _is_elasticity_tensor_guaranteed_isotropic =
      hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC);

  std::vector<MaterialName> models = getParam<std::vector<MaterialName>>("inelastic_models");
  for (unsigned int i = 0; i < _num_models; ++i)
  {
    StressUpdateBase * rrr = dynamic_cast<StressUpdateBase *>(&getMaterialByName(models[i]));
    if (rrr)
    {
      _models.push_back(rrr);
      if (rrr->requiresIsotropicTensor() && !_is_elasticity_tensor_guaranteed_isotropic)
        mooseError("Model " + models[i] +
                   " requires an isotropic elasticity tensor, but the one supplied is not "
                   "guaranteed isotropic");
    }
    else
      mooseError("Model " + models[i] + " is not compatible with ComputeMultipleInelasticStress");
  }
}

void
ComputeMultipleInelasticStress::computeQpStress()
{
  computeQpStressIntermediateConfiguration();
  if (_perform_finite_strain_rotations)
    finiteStrainRotation();
}

void
ComputeMultipleInelasticStress::computeQpStressIntermediateConfiguration()
{
  RankTwoTensor elastic_strain_increment;
  RankTwoTensor combined_inelastic_strain_increment;

  if (_num_models == 0)
  {
    _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp];

    // If the elasticity tensor values have changed and the tensor is isotropic,
    // use the old strain to calculate the old stress
    if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
    {
      _stress[_qp] = _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + _strain_increment[_qp]);
      // InitialStress Deprecation: remove these lines
      if (_perform_finite_strain_rotations)
        rotateQpInitialStress();
      addQpInitialStress();
    }
    else
      _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * _strain_increment[_qp];

    if (_fe_problem.currentlyComputingJacobian())
      _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
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
ComputeMultipleInelasticStress::finiteStrainRotation(const bool force_elasticity_rotation)
{
  _elastic_strain[_qp] =
      _rotation_increment[_qp] * _elastic_strain[_qp] * _rotation_increment[_qp].transpose();
  _stress[_qp] = _rotation_increment[_qp] * _stress[_qp] * _rotation_increment[_qp].transpose();
  _inelastic_strain[_qp] =
      _rotation_increment[_qp] * _inelastic_strain[_qp] * _rotation_increment[_qp].transpose();
  if (force_elasticity_rotation ||
      !(_is_elasticity_tensor_guaranteed_isotropic &&
        (_tangent_operator_type == TangentOperatorEnum::elastic || _num_models == 0)))
    _Jacobian_mult[_qp].rotate(_rotation_increment[_qp]);
}

void
ComputeMultipleInelasticStress::updateQpState(RankTwoTensor & elastic_strain_increment,
                                              RankTwoTensor & combined_inelastic_strain_increment)
{
  if (_output_iteration_info == true)
  {
    _console << std::endl
             << "iteration output for ComputeMultipleInelasticStress solve:"
             << " time=" << _t << " int_pt=" << _qp << std::endl;
  }
  Real l2norm_delta_stress, first_l2norm_delta_stress;
  unsigned int counter = 0;

  std::vector<RankTwoTensor> inelastic_strain_increment;
  inelastic_strain_increment.resize(_num_models);

  for (unsigned i_rmm = 0; i_rmm < _models.size(); ++i_rmm)
    inelastic_strain_increment[i_rmm].zero();

  RankTwoTensor stress_max, stress_min;

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
      {
        _stress[_qp] =
            _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
        // InitialStress Deprecation: remove these lines
        if (_perform_finite_strain_rotations)
          rotateQpInitialStress();
        addQpInitialStress();
      }
      else
        _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;

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
        for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
        {
          for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
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
    RankTwoTensor delta_stress(stress_max - stress_min);
    l2norm_delta_stress = delta_stress.L2norm();
    if (counter == 0)
      first_l2norm_delta_stress = l2norm_delta_stress;

    if (_output_iteration_info == true)
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
  for (unsigned i_rmm = 0; i_rmm < _num_models; ++i_rmm)
    combined_inelastic_strain_increment +=
        _inelastic_weights[i_rmm] * inelastic_strain_increment[i_rmm];

  if (_fe_problem.currentlyComputingJacobian())
    computeQpJacobianMult();

  _matl_timestep_limit[_qp] = 0.0;
  for (unsigned i_rmm = 0; i_rmm < _num_models; ++i_rmm)
    _matl_timestep_limit[_qp] += 1.0 / _models[i_rmm]->computeTimeStepLimit();

  if (MooseUtils::absoluteFuzzyEqual(_matl_timestep_limit[_qp], 0.0))
  {
    _matl_timestep_limit[_qp] = std::numeric_limits<Real>::max();
  }
  else
  {
    _matl_timestep_limit[_qp] = 1.0 / _matl_timestep_limit[_qp];
  }
}

void
ComputeMultipleInelasticStress::computeQpJacobianMult()
{
  if (_tangent_operator_type == TangentOperatorEnum::elastic)
    _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
  else
  {
    const RankFourTensor E_inv = _elasticity_tensor[_qp].invSymm();
    _Jacobian_mult[_qp] = _consistent_tangent_operator[0];
    for (unsigned i_rmm = 1; i_rmm < _num_models; ++i_rmm)
      _Jacobian_mult[_qp] = _consistent_tangent_operator[i_rmm] * E_inv * _Jacobian_mult[_qp];
  }
}

void
ComputeMultipleInelasticStress::updateQpStateSingleModel(
    unsigned model_number,
    RankTwoTensor & elastic_strain_increment,
    RankTwoTensor & combined_inelastic_strain_increment)
{
  for (auto model : _models)
    model->setQp(_qp);

  elastic_strain_increment = _strain_increment[_qp];

  // If the elasticity tensor values have changed and the tensor is isotropic,
  // use the old strain to calculate the old stress
  if (_is_elasticity_tensor_guaranteed_isotropic || !_perform_finite_strain_rotations)
  {
    _stress[_qp] = _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + elastic_strain_increment);
    // InitialStress Deprecation: remove these lines
    if (_perform_finite_strain_rotations)
      rotateQpInitialStress();
    addQpInitialStress();
  }
  else
    _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;

  computeAdmissibleState(model_number,
                         elastic_strain_increment,
                         combined_inelastic_strain_increment,
                         _Jacobian_mult[_qp]);

  _matl_timestep_limit[_qp] = _models[0]->computeTimeStepLimit();

  /* propagate internal variables, etc, to this timestep for those inelastic models where
   * "updateState" is not called */
  for (unsigned i_rmm = 0; i_rmm < _num_models; ++i_rmm)
    if (i_rmm != model_number)
      _models[i_rmm]->propagateQpStatefulProperties();
}

void
ComputeMultipleInelasticStress::computeAdmissibleState(unsigned model_number,
                                                       RankTwoTensor & elastic_strain_increment,
                                                       RankTwoTensor & inelastic_strain_increment,
                                                       RankFourTensor & consistent_tangent_operator)
{
  _models[model_number]->updateState(elastic_strain_increment,
                                     inelastic_strain_increment,
                                     _rotation_increment[_qp],
                                     _stress[_qp],
                                     _stress_old[_qp],
                                     _elasticity_tensor[_qp],
                                     _elastic_strain_old[_qp],
                                     _tangent_operator_type == TangentOperatorEnum::nonlinear,
                                     consistent_tangent_operator);
}
