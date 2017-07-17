/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                        Grizzly                               */
/*                                                              */
/*           (c) 2015 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GeneralizedKelvinVoigtBase.h"

template<>
InputParameters
validParams<GeneralizedKelvinVoigtBase>()
{
  InputParameters params = validParams<LinearViscoelasticityBase>();
  return params;
}

GeneralizedKelvinVoigtBase::GeneralizedKelvinVoigtBase(const InputParameters & parameters)
  : LinearViscoelasticityBase(parameters)
{
  _need_viscoelastic_properties_inverse = true;
}

void
GeneralizedKelvinVoigtBase::updateQpApparentProperties(unsigned int qp,
                                                       const RankTwoTensor & /*effective_strain*/,
                                                       const RankTwoTensor & effective_stress)
{
  for (unsigned int i = 0; i < _springs_elasticity_tensors[qp].size(); ++i)
  {
    Real theta_i = computeTheta(_dt, _dashpot_viscosities[qp][i]);
    Real gamma = _dashpot_viscosities[qp][i] / (_dt * theta_i);
    _viscous_strains[qp][i] = ((*_springs_elasticity_tensors_inv)[qp][i] * effective_stress) / (theta_i * (1. + gamma));
    _viscous_strains[qp][i] += _viscous_strains_old[qp][i] * (gamma / (theta_i * (1. + gamma)) - (1. - theta_i) / theta_i);
  }

  if (_has_longterm_dashpot)
  {
    _viscous_strains[qp].back() = ((*_first_elasticity_tensor_inv)[qp] * effective_stress) * (_dt / _dashpot_viscosities[qp].back());
    _viscous_strains[qp].back() += _viscous_strains_old[qp].back();
  }

  if (_has_driving_eigenstrain)
  {
    RankTwoTensor driving_effective_stress = _instantaneous_elasticity_tensor[qp] * (*_driving_eigenstrain)[qp];

    for (unsigned int i = 0; i < _springs_elasticity_tensors[qp].size(); ++i)
    {
      Real theta_i = computeTheta(_dt, _dashpot_viscosities[qp][i]);
      Real gamma = _dashpot_viscosities[qp][i] / (_dt * theta_i);
      _viscous_strains[qp][i] += ((*_springs_elasticity_tensors_inv)[qp][i] * driving_effective_stress) / (theta_i * (1. + gamma));
    }

    if (_has_longterm_dashpot)
      _viscous_strains[qp].back() += ((*_first_elasticity_tensor_inv)[qp] * driving_effective_stress) * (_dt / _dashpot_viscosities[qp].back());
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
    _apparent_elasticity_tensor_inv[_qp] += (*_springs_elasticity_tensors_inv)[_qp][i] / (1. + gamma);
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
    _apparent_creep_strain[_qp] += 
        (_instantaneous_elasticity_tensor[_qp] * _apparent_elasticity_tensor_inv[_qp]) *
        (*_driving_eigenstrain)[_qp];
  }
}


