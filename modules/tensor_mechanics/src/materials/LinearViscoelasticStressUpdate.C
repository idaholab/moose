//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearViscoelasticStressUpdate.h"

template <>
InputParameters
validParams<LinearViscoelasticStressUpdate>()
{
  InputParameters params = validParams<StressUpdateBase>();
  params.addParam<std::string>("base_name", "optional string prepended to the creep strain name");
  params.addParam<std::string>(
      "apparent_creep_strain",
      "apparent_creep_strain",
      "name of the apparent creep strain (defined by a LinearViscoelasticityBase material)");
  params.addParam<std::string>(
      "apparent_elasticity_tensor",
      "apparent_elasticity_tensor",
      "name of the apparent elasticity tensor (defined by a LinearViscoelasticityBase material)");
  params.addParam<std::string>(
      "instantaneous_elasticity_tensor_inv",
      "instantaneous_elasticity_tensor_inv",
      "name of the apparent compliance tensor (defined by a LinearViscoelasticityBase material)");
  return params;
}

LinearViscoelasticStressUpdate::LinearViscoelasticStressUpdate(const InputParameters & parameters)
  : StressUpdateBase(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") : std::string()),
    _creep_strain(declareProperty<RankTwoTensor>(
        isParamValid("base_name") ? _base_name + "_creep_strain" : "creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<RankTwoTensor>(
        isParamValid("base_name") ? _base_name + "_creep_strain" : "creep_strain")),
    _apparent_creep_strain(getMaterialProperty<RankTwoTensor>("apparent_creep_strain")),
    _apparent_elasticity_tensor(getMaterialProperty<RankFourTensor>("apparent_elasticity_tensor")),
    _instantaneous_elasticity_tensor_inv(
        getMaterialProperty<RankFourTensor>("instantaneous_elasticity_tensor_inv"))
{
}

void
LinearViscoelasticStressUpdate::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();
}

void
LinearViscoelasticStressUpdate::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];
}

void
LinearViscoelasticStressUpdate::updateState(RankTwoTensor & strain_increment,
                                            RankTwoTensor & inelastic_strain_increment,
                                            const RankTwoTensor & /*rotation_increment*/,
                                            RankTwoTensor & stress_new,
                                            const RankTwoTensor & /*stress_old*/,
                                            const RankFourTensor & elasticity_tensor,
                                            const RankTwoTensor & elastic_strain_old,
                                            bool /*compute_full_tangent_operator*/,
                                            RankFourTensor & tangent_operator)
{
  RankTwoTensor current_mechanical_strain =
      elastic_strain_old + _creep_strain_old[_qp] + strain_increment;

  _creep_strain[_qp] =
      current_mechanical_strain -
      (_apparent_elasticity_tensor[_qp] * _instantaneous_elasticity_tensor_inv[_qp]) *
          (current_mechanical_strain - _apparent_creep_strain[_qp]);

  RankTwoTensor creep_strain_increment = _creep_strain[_qp] - _creep_strain_old[_qp];

  strain_increment -= creep_strain_increment;
  inelastic_strain_increment += creep_strain_increment;
  stress_new -= elasticity_tensor * creep_strain_increment;

  tangent_operator = elasticity_tensor;
}
