//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedKelvinVoigtBase.h"

template <>
InputParameters
validParams<GeneralizedKelvinVoigtBase>()
{
  InputParameters params = validParams<LinearViscoelasticityBase>();
  params.set<bool>("need_viscoelastic_properties_inverse") = true;
  params.suppressParameter<bool>("need_viscoelastic_properties_inverse");
  return params;
}

GeneralizedKelvinVoigtBase::GeneralizedKelvinVoigtBase(const InputParameters & parameters)
  : LinearViscoelasticityBase(parameters)
{
  _need_viscoelastic_properties_inverse = true;
}

void
GeneralizedKelvinVoigtBase::updateQpViscousStrains()
{
  RankTwoTensor effective_stress = _stress_old[_qp];
  if (_has_driving_eigenstrain)
    effective_stress += _instantaneous_elasticity_tensor[_qp] * (*_driving_eigenstrain)[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
  {
    Real theta_i = computeTheta(_dt_old, _dashpot_viscosities[_qp][i]);
    Real gamma = _dashpot_viscosities[_qp][i] / (_dt_old * theta_i);
    _viscous_strains[_qp][i] =
        ((*_springs_elasticity_tensors_inv)[_qp][i] * effective_stress) / (theta_i * (1. + gamma));
    _viscous_strains[_qp][i] += _viscous_strains_old[_qp][i] *
                                (gamma / (theta_i * (1. + gamma)) - (1. - theta_i) / theta_i);
  }

  if (_has_longterm_dashpot)
  {
    _viscous_strains[_qp].back() = ((*_first_elasticity_tensor_inv)[_qp] * effective_stress) *
                                   (_dt_old / _dashpot_viscosities[_qp].back());
    _viscous_strains[_qp].back() += _viscous_strains_old[_qp].back();
  }
}

void
GeneralizedKelvinVoigtBase::computeQpApparentElasticityTensors()
{
  _instantaneous_elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];
  _instantaneous_elasticity_tensor_inv[_qp] = (*_first_elasticity_tensor_inv)[_qp];
  _apparent_elasticity_tensor_inv[_qp] = (*_first_elasticity_tensor_inv)[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
  {
    Real theta_i = computeTheta(_dt, _dashpot_viscosities[_qp][i]);
    Real gamma = _dashpot_viscosities[_qp][i] / (_dt * theta_i);
    _apparent_elasticity_tensor_inv[_qp] +=
        (*_springs_elasticity_tensors_inv)[_qp][i] / (1. + gamma);
  }

  if (_has_longterm_dashpot)
  {
    Real theta_i = computeTheta(_dt, _dashpot_viscosities[_qp].back());
    Real gamma = _dashpot_viscosities[_qp].back() / (_dt * theta_i);
    _apparent_elasticity_tensor_inv[_qp] += (*_first_elasticity_tensor_inv)[_qp] / gamma;
  }

  _apparent_elasticity_tensor[_qp] = _apparent_elasticity_tensor_inv[_qp].invSymm();
}

void
GeneralizedKelvinVoigtBase::computeQpApparentCreepStrain()
{
  _apparent_creep_strain[_qp].zero();

  for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
  {
    Real theta_i = computeTheta(_dt, _dashpot_viscosities[_qp][i]);
    Real gamma = _dashpot_viscosities[_qp][i] / (_dt * theta_i);
    _apparent_creep_strain[_qp] += _viscous_strains[_qp][i] * (gamma / (1. + gamma));
  }

  if (_has_longterm_dashpot)
    _apparent_creep_strain[_qp] += _viscous_strains[_qp].back();

  if (_has_driving_eigenstrain)
  {
    RankFourTensor cumulated_driving_tensor;
    cumulated_driving_tensor.zero();
    for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
    {
      double theta_i = computeTheta(_dt, _dashpot_viscosities[_qp][i]);
      double gamma = _dashpot_viscosities[_qp][i] / (_dt * theta_i);
      cumulated_driving_tensor += (*_springs_elasticity_tensors_inv)[_qp][i] / (1. + gamma);
    }

    _apparent_creep_strain[_qp] +=
        (_instantaneous_elasticity_tensor[_qp] * cumulated_driving_tensor) *
        (*_driving_eigenstrain)[_qp];

    if (_has_longterm_dashpot)
    {
      double theta_i = computeTheta(_dt, _dashpot_viscosities[_qp].back());
      double gamma = _dashpot_viscosities[_qp].back() / (_dt * theta_i);
      _apparent_creep_strain[_qp] += (*_driving_eigenstrain)[_qp] / gamma;
    }
  }
}
