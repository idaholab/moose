//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedMaxwellBase.h"

InputParameters
GeneralizedMaxwellBase::validParams()
{
  InputParameters params = LinearViscoelasticityBase::validParams();
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
  if (_t_step <= 1)
    return;

  RankTwoTensor effective_strain = _elastic_strain_old[_qp] + _creep_strain_old[_qp];
  if (_has_driving_eigenstrain)
    effective_strain += (*_driving_eigenstrain_old)[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors.size(); ++i)
  {
    Real theta_i = computeTheta(_dt_old, (*_dashpot_viscosities_old[i])[_qp]);
    Real gamma = (*_dashpot_viscosities_old[i])[_qp] / (_dt_old * theta_i);
    (*_viscous_strains[i])[_qp] =
        (*_viscous_strains_old[i])[_qp] *
        (((*_dashpot_viscosities_old[i])[_qp] * gamma - _dt_old * (1. - theta_i) * gamma) /
         ((*_dashpot_viscosities_old[i])[_qp] * (1. + gamma)));
    (*_viscous_strains[i])[_qp] +=
        effective_strain *
        (((*_dashpot_viscosities_old[i])[_qp] + _dt_old * (1. - theta_i) * gamma) /
         ((*_dashpot_viscosities_old[i])[_qp] * (1. + gamma)));
  }

  if (_has_longterm_dashpot)
  {
    Real theta_i = computeTheta(_dt_old, (*_dashpot_viscosities_old.back())[_qp]);
    (*_viscous_strains.back())[_qp] = effective_strain / theta_i;
    (*_viscous_strains.back())[_qp] -=
        (*_viscous_strains_old.back())[_qp] * ((1. - theta_i) / theta_i);
  }
}

void
GeneralizedMaxwellBase::computeQpApparentElasticityTensors()
{

  _elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];
  _apparent_elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors.size(); ++i)
  {
    Real theta_i = computeTheta(_dt, (*_dashpot_viscosities[i])[_qp]);
    Real gamma = (*_dashpot_viscosities[i])[_qp] / (_dt * theta_i);
    _elasticity_tensor[_qp] += (*_springs_elasticity_tensors[i])[_qp];
    _apparent_elasticity_tensor[_qp] +=
        (*_springs_elasticity_tensors[i])[_qp] * (gamma / (1. + gamma));
  }

  if (_has_longterm_dashpot)
  {
    Real theta_i = computeTheta(_dt, (*_dashpot_viscosities.back())[_qp]);
    Real gamma = (*_dashpot_viscosities.back())[_qp] / (_dt * theta_i);
    _apparent_elasticity_tensor[_qp] += _first_elasticity_tensor[_qp] * gamma;

    mooseDoOnce(mooseWarning("Generalized Maxwell model with longterm viscosity may not converge "
                             "under Dirichlet boundary conditions"));
  }

  _apparent_elasticity_tensor_inv[_qp] = _apparent_elasticity_tensor[_qp].invSymm();
  _elasticity_tensor_inv[_qp] = _elasticity_tensor[_qp].invSymm();
}

void
GeneralizedMaxwellBase::computeQpApparentCreepStrain()
{
  _apparent_creep_strain[_qp].zero();

  for (unsigned int i = 0; i < _springs_elasticity_tensors.size(); ++i)
  {
    Real theta_i = computeTheta(_dt, (*_dashpot_viscosities[i])[_qp]);
    Real gamma = (*_dashpot_viscosities[i])[_qp] / (_dt * theta_i);
    _apparent_creep_strain[_qp] +=
        ((*_springs_elasticity_tensors[i])[_qp] * (*_viscous_strains[i])[_qp]) *
        (gamma / (1. + gamma));
  }

  if (_has_longterm_dashpot)
  {
    Real theta_i = computeTheta(_dt, (*_dashpot_viscosities.back())[_qp]);
    Real gamma = (*_dashpot_viscosities.back())[_qp] / (_dt * theta_i);
    _apparent_creep_strain[_qp] +=
        (_first_elasticity_tensor[_qp] * (*_viscous_strains.back())[_qp]) * gamma;
  }

  _apparent_creep_strain[_qp] = _apparent_elasticity_tensor_inv[_qp] * _apparent_creep_strain[_qp];

  if (_has_driving_eigenstrain)
  {
    _apparent_creep_strain[_qp] +=
        (_elasticity_tensor[_qp] * _apparent_elasticity_tensor_inv[_qp]) *
        (*_driving_eigenstrain)[_qp];
    _apparent_creep_strain[_qp] -= (*_driving_eigenstrain)[_qp];
  }
}
