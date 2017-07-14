/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GeneralizedMaxwellBase.h"

template <>
InputParameters
validParams<GeneralizedMaxwellBase>()
{
  InputParameters params = validParams<LinearViscoelasticityBase>();
  return params;
}

GeneralizedMaxwellBase::GeneralizedMaxwellBase(const InputParameters & parameters)
  : LinearViscoelasticityBase(parameters), _has_been_warned(false)
{
}

void
GeneralizedMaxwellBase::emitWarning()
{
  mooseWarning("Generalized Maxwell model with longterm viscosity may not converge under Dirichlet "
               "boundary conditions");
  _has_been_warned = true;
}

void
GeneralizedMaxwellBase::computeQpApparentElasticityTensors()
{

  _instantaneous_elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];
  _apparent_elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];

  for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[_qp][i]);
    double gamma = _dashpot_viscosities[_qp][i] / (_dt * theta_i);
    _instantaneous_elasticity_tensor[_qp] += _springs_elasticity_tensors[_qp][i];
    _apparent_elasticity_tensor[_qp] +=
        _springs_elasticity_tensors[_qp][i] * (gamma / (1. + gamma));
  }

  if (hasLongtermDashpot(_qp))
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[_qp].back());
    double gamma = _dashpot_viscosities[_qp].back() / (_dt * theta_i);
    _apparent_elasticity_tensor[_qp] += _first_elasticity_tensor[_qp] * gamma;

    if (!_has_been_warned)
      emitWarning();
  }
}

void
GeneralizedMaxwellBase::accumulateQpViscousStrain(
    unsigned int qp,
    RankTwoTensor & accumulated_viscous_strain,
    const std::vector<RankTwoTensor> & viscous_strains,
    bool has_driving_eigenstrain,
    const RankTwoTensor & driving_eigenstrain) const
{

  accumulated_viscous_strain.zero();

  for (unsigned int i = 0; i < _springs_elasticity_tensors[qp].size(); ++i)
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[qp][i]);
    double gamma = _dashpot_viscosities[qp][i] / (_dt * theta_i);
    accumulated_viscous_strain +=
        (_springs_elasticity_tensors[qp][i] * viscous_strains[i]) * (gamma / (1. + gamma));
  }

  if (hasLongtermDashpot(qp))
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[qp].back());
    double gamma = _dashpot_viscosities[qp].back() / (_dt * theta_i);
    accumulated_viscous_strain += (_first_elasticity_tensor[qp] * viscous_strains.back()) * gamma;
  }

  accumulated_viscous_strain =
      _apparent_elasticity_tensor[qp].invSymm() * accumulated_viscous_strain;

  if (has_driving_eigenstrain)
  {
    accumulated_viscous_strain +=
        (_instantaneous_elasticity_tensor[qp] * _apparent_elasticity_tensor[qp].invSymm()) *
        driving_eigenstrain;
    accumulated_viscous_strain -= driving_eigenstrain;
  }
}

void
GeneralizedMaxwellBase::updateQpViscousStrain(
    unsigned int qp,
    std::vector<RankTwoTensor> & viscous_strains,
    const std::vector<RankTwoTensor> & viscous_strains_old,
    const RankTwoTensor & effective_strain,
    const RankTwoTensor & /*effective_stress*/,
    bool has_driving_eigenstrain,
    const RankTwoTensor & driving_eigenstrain) const
{
  for (unsigned int i = 0; i < _springs_elasticity_tensors[qp].size(); ++i)
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[qp][i]);
    double gamma = _dashpot_viscosities[qp][i] / (_dt * theta_i);
    viscous_strains[i] = viscous_strains_old[i] *
                         ((_dashpot_viscosities[qp][i] * gamma - _dt * (1. - theta_i) * gamma) /
                          (_dashpot_viscosities[qp][i] * (1. + gamma)));
    viscous_strains[i] +=
        effective_strain * ((_dashpot_viscosities[qp][i] + _dt * (1. - theta_i) * gamma) /
                            (_dashpot_viscosities[qp][i] * (1. + gamma)));
  }

  if (hasLongtermDashpot(qp))
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[qp].back());
    viscous_strains.back() = effective_strain / theta_i;
    viscous_strains.back() -= viscous_strains_old.back() * ((1. - theta_i) / theta_i);
  }

  if (has_driving_eigenstrain)
  {
    for (unsigned int i = 0; i < _springs_elasticity_tensors[qp].size(); ++i)
    {
      double theta_i = computeTheta(_dt, _dashpot_viscosities[qp][i]);
      double gamma = _dashpot_viscosities[qp][i] / (_dt * theta_i);
      viscous_strains[i] +=
          driving_eigenstrain * ((_dashpot_viscosities[qp][i] + _dt * (1. - theta_i) * gamma) /
                                 (_dashpot_viscosities[qp][i] * (1. + gamma)));
    }

    if (hasLongtermDashpot(qp))
    {
      double theta_i = computeTheta(_dt, _dashpot_viscosities[qp].back());
      viscous_strains.back() += driving_eigenstrain / theta_i;
    }
  }
}
