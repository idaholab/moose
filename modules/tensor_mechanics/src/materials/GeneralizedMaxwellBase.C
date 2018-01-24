//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedMaxwellBase.h"

template <>
InputParameters
validParams<GeneralizedMaxwellBase>()
{
  InputParameters params = validParams<LinearViscoelasticityBase>();
  return params;
}

GeneralizedMaxwellBase::GeneralizedMaxwellBase(const InputParameters & parameters)
  : LinearViscoelasticityBase(parameters)
{
  _need_viscoelastic_properties_inverse = false;
}

void
GeneralizedMaxwellBase::updateQpViscousStrains()
{
  RankTwoTensor effective_strain = _elastic_strain_old[_qp] + _creep_strain_old[_qp];
  if (_has_driving_eigenstrain)
    effective_strain += (*_driving_eigenstrain)[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
  {
    Real theta_i = computeTheta(_dt_old, _dashpot_viscosities[_qp][i]);
    Real gamma = _dashpot_viscosities[_qp][i] / (_dt_old * theta_i);
    _viscous_strains[_qp][i] =
        _viscous_strains_old[_qp][i] *
        ((_dashpot_viscosities[_qp][i] * gamma - _dt_old * (1. - theta_i) * gamma) /
         (_dashpot_viscosities[_qp][i] * (1. + gamma)));
    _viscous_strains[_qp][i] +=
        effective_strain * ((_dashpot_viscosities[_qp][i] + _dt_old * (1. - theta_i) * gamma) /
                            (_dashpot_viscosities[_qp][i] * (1. + gamma)));
  }

  if (_has_longterm_dashpot)
  {
    Real theta_i = computeTheta(_dt_old, _dashpot_viscosities[_qp].back());
    _viscous_strains[_qp].back() = effective_strain / theta_i;
    _viscous_strains[_qp].back() -= _viscous_strains_old[_qp].back() * ((1. - theta_i) / theta_i);
  }
}

void
GeneralizedMaxwellBase::computeQpApparentElasticityTensors()
{

  _instantaneous_elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];
  _apparent_elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
  {
    Real theta_i = computeTheta(_dt, _dashpot_viscosities[_qp][i]);
    Real gamma = _dashpot_viscosities[_qp][i] / (_dt * theta_i);
    _instantaneous_elasticity_tensor[_qp] += _springs_elasticity_tensors[_qp][i];
    _apparent_elasticity_tensor[_qp] +=
        _springs_elasticity_tensors[_qp][i] * (gamma / (1. + gamma));
  }

  if (_has_longterm_dashpot)
  {
    Real theta_i = computeTheta(_dt, _dashpot_viscosities[_qp].back());
    Real gamma = _dashpot_viscosities[_qp].back() / (_dt * theta_i);
    _apparent_elasticity_tensor[_qp] += _first_elasticity_tensor[_qp] * gamma;

    mooseDoOnce(mooseWarning("Generalized Maxwell model with longterm viscosity may not converge "
                             "under Dirichlet boundary conditions"));
  }

  _apparent_elasticity_tensor_inv[_qp] = _apparent_elasticity_tensor[_qp].invSymm();
  _instantaneous_elasticity_tensor_inv[_qp] = _instantaneous_elasticity_tensor[_qp].invSymm();
}

void
GeneralizedMaxwellBase::computeQpApparentCreepStrain()
{
  _apparent_creep_strain[_qp].zero();

  for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
  {
    Real theta_i = computeTheta(_dt, _dashpot_viscosities[_qp][i]);
    Real gamma = _dashpot_viscosities[_qp][i] / (_dt * theta_i);
    _apparent_creep_strain[_qp] +=
        (_springs_elasticity_tensors[_qp][i] * _viscous_strains[_qp][i]) * (gamma / (1. + gamma));
  }

  if (_has_longterm_dashpot)
  {
    Real theta_i = computeTheta(_dt, _dashpot_viscosities[_qp].back());
    Real gamma = _dashpot_viscosities[_qp].back() / (_dt * theta_i);
    _apparent_creep_strain[_qp] +=
        (_first_elasticity_tensor[_qp] * _viscous_strains[_qp].back()) * gamma;
  }

  _apparent_creep_strain[_qp] = _apparent_elasticity_tensor_inv[_qp] * _apparent_creep_strain[_qp];

  if (_has_driving_eigenstrain)
  {
    _apparent_creep_strain[_qp] +=
        (_instantaneous_elasticity_tensor[_qp] * _apparent_elasticity_tensor_inv[_qp]) *
        (*_driving_eigenstrain)[_qp];
    _apparent_creep_strain[_qp] -= (*_driving_eigenstrain)[_qp];
  }
}
