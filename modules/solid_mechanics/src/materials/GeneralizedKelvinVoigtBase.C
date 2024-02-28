//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedKelvinVoigtBase.h"

InputParameters
GeneralizedKelvinVoigtBase::validParams()
{
  InputParameters params = LinearViscoelasticityBase::validParams();
  params.set<bool>("need_viscoelastic_properties_inverse") = true;
  params.suppressParameter<bool>("need_viscoelastic_properties_inverse");
  return params;
}

GeneralizedKelvinVoigtBase::GeneralizedKelvinVoigtBase(const InputParameters & parameters)
  : LinearViscoelasticityBase(parameters),
    _first_elasticity_tensor_old(
        getMaterialPropertyOld<RankFourTensor>(_base_name + "spring_elasticity_tensor_0")),
    _first_elasticity_tensor_inv_old(
        getMaterialPropertyOld<RankFourTensor>(_base_name + "spring_elasticity_tensor_0_inv"))
{
}

void
GeneralizedKelvinVoigtBase::updateQpViscousStrains()
{
  if (_t_step <= 1)
    return;

  RankTwoTensor effective_stress = _first_elasticity_tensor_old[_qp] * _elastic_strain_old[_qp];

  if (_has_driving_eigenstrain)
    effective_stress += _first_elasticity_tensor_old[_qp] * (*_driving_eigenstrain_old)[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors.size(); ++i)
  {
    Real theta_i = computeTheta(_dt_old, (*_dashpot_viscosities_old[i])[_qp]);
    Real gamma = (*_dashpot_viscosities_old[i])[_qp] / (_dt_old * theta_i);
    (*_viscous_strains[i])[_qp] =
        ((*_springs_elasticity_tensors_inv_old[i])[_qp] * effective_stress) /
        (theta_i * (1. + gamma));
    (*_viscous_strains[i])[_qp] += (*_viscous_strains_old[i])[_qp] *
                                   (gamma / (theta_i * (1. + gamma)) - (1. - theta_i) / theta_i);
  }

  if (_has_longterm_dashpot)
  {
    (*_viscous_strains.back())[_qp] = (_first_elasticity_tensor_inv_old[_qp] * effective_stress) *
                                      (_dt_old / (*_dashpot_viscosities_old.back())[_qp]);
    (*_viscous_strains.back())[_qp] += (*_viscous_strains_old.back())[_qp];
  }
}

void
GeneralizedKelvinVoigtBase::computeQpApparentElasticityTensors()
{
  _elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];
  _elasticity_tensor_inv[_qp] = (*_first_elasticity_tensor_inv)[_qp];
  _apparent_elasticity_tensor_inv[_qp] = (*_first_elasticity_tensor_inv)[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors.size(); ++i)
  {
    Real theta_i = computeTheta(_dt, (*_dashpot_viscosities[i])[_qp]);
    Real gamma = (*_dashpot_viscosities[i])[_qp] / (_dt * theta_i);
    _apparent_elasticity_tensor_inv[_qp] +=
        (*_springs_elasticity_tensors_inv[i])[_qp] / (1. + gamma);
  }

  if (_has_longterm_dashpot)
  {
    Real theta_i = computeTheta(_dt, (*_dashpot_viscosities.back())[_qp]);
    Real gamma = (*_dashpot_viscosities.back())[_qp] / (_dt * theta_i);
    _apparent_elasticity_tensor_inv[_qp] += (*_first_elasticity_tensor_inv)[_qp] / gamma;
  }

  _apparent_elasticity_tensor[_qp] = _apparent_elasticity_tensor_inv[_qp].invSymm();
}

void
GeneralizedKelvinVoigtBase::computeQpApparentCreepStrain()
{
  _apparent_creep_strain[_qp].zero();

  for (unsigned int i = 0; i < _springs_elasticity_tensors.size(); ++i)
  {
    Real theta_i = computeTheta(_dt, (*_dashpot_viscosities[i])[_qp]);
    Real gamma = (*_dashpot_viscosities[i])[_qp] / (_dt * theta_i);
    _apparent_creep_strain[_qp] += (*_viscous_strains[i])[_qp] * (gamma / (1. + gamma));
  }

  if (_has_longterm_dashpot)
    _apparent_creep_strain[_qp] += (*_viscous_strains.back())[_qp];

  if (_has_driving_eigenstrain)
  {
    RankFourTensor cumulated_driving_tensor;
    cumulated_driving_tensor.zero();
    for (unsigned int i = 0; i < _springs_elasticity_tensors.size(); ++i)
    {
      double theta_i = computeTheta(_dt, (*_dashpot_viscosities[i])[_qp]);
      double gamma = (*_dashpot_viscosities[i])[_qp] / (_dt * theta_i);
      cumulated_driving_tensor += (*_springs_elasticity_tensors_inv[i])[_qp] / (1. + gamma);
    }

    _apparent_creep_strain[_qp] +=
        (_elasticity_tensor[_qp] * cumulated_driving_tensor) * (*_driving_eigenstrain)[_qp];

    if (_has_longterm_dashpot)
    {
      double theta_i = computeTheta(_dt, (*_dashpot_viscosities.back())[_qp]);
      double gamma = (*_dashpot_viscosities.back())[_qp] / (_dt * theta_i);
      _apparent_creep_strain[_qp] += (*_driving_eigenstrain)[_qp] / gamma;
    }
  }
}
