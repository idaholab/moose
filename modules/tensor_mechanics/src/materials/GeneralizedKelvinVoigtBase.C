/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GeneralizedKelvinVoigtBase.h"

template <>
InputParameters
validParams<GeneralizedKelvinVoigtBase>()
{
  InputParameters params = validParams<LinearViscoelasticityBase>();
  return params;
}

GeneralizedKelvinVoigtBase::GeneralizedKelvinVoigtBase(const InputParameters & parameters)
  : LinearViscoelasticityBase(parameters)
{
}

void
GeneralizedKelvinVoigtBase::computeQpApparentElasticityTensors()
{
  _instantaneous_elasticity_tensor[_qp] = _first_elasticity_tensor[_qp];
  _apparent_elasticity_tensor[_qp] = _first_elasticity_tensor[_qp].invSymm();

  for (unsigned int i = 0; i < _springs_elasticity_tensors[_qp].size(); ++i)
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[_qp][i]);
    double gamma = _dashpot_viscosities[_qp][i] / (_dt * theta_i);
    _apparent_elasticity_tensor[_qp] +=
        _springs_elasticity_tensors[_qp][i].invSymm() / (1. + gamma);
  }

  if (hasLongtermDashpot(_qp))
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[_qp].back());
    double gamma = _dashpot_viscosities[_qp].back() / (_dt * theta_i);
    _apparent_elasticity_tensor[_qp] += _first_elasticity_tensor[_qp].invSymm() / gamma;
  }

  _apparent_elasticity_tensor[_qp] = _apparent_elasticity_tensor[_qp].invSymm();
}

void
GeneralizedKelvinVoigtBase::accumulateQpViscousStrain(
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
    accumulated_viscous_strain += viscous_strains[i] * (gamma / (1. + gamma));
  }

  if (hasLongtermDashpot(qp))
    accumulated_viscous_strain += viscous_strains.back();

  if (has_driving_eigenstrain)
  {
    RankFourTensor cumulated_driving_tensor;
    cumulated_driving_tensor.zero();
    for (unsigned int i = 0; i < _springs_elasticity_tensors[qp].size(); ++i)
    {
      double theta_i = computeTheta(_dt, _dashpot_viscosities[qp][i]);
      double gamma = _dashpot_viscosities[qp][i] / (_dt * theta_i);
      cumulated_driving_tensor += _springs_elasticity_tensors[qp][i].invSymm() / (1. + gamma);
    }

    cumulated_driving_tensor = _instantaneous_elasticity_tensor[qp] * cumulated_driving_tensor;
    accumulated_viscous_strain += cumulated_driving_tensor * driving_eigenstrain;

    if (hasLongtermDashpot(qp))
    {
      double theta_i = computeTheta(_dt, _dashpot_viscosities[qp].back());
      double gamma = _dashpot_viscosities[qp].back() / (_dt * theta_i);
      accumulated_viscous_strain += driving_eigenstrain / gamma;
    }
  }
}

void
GeneralizedKelvinVoigtBase::updateQpViscousStrain(
    unsigned int qp,
    std::vector<RankTwoTensor> & viscous_strains,
    const std::vector<RankTwoTensor> & viscous_strains_old,
    const RankTwoTensor & /*effective_strain*/,
    const RankTwoTensor & effective_stress,
    bool has_driving_eigenstrain,
    const RankTwoTensor & driving_eigenstrain) const
{

  for (unsigned int i = 0; i < _springs_elasticity_tensors[qp].size(); ++i)
  {
    double theta_i = computeTheta(_dt, _dashpot_viscosities[qp][i]);
    double gamma = _dashpot_viscosities[qp][i] / (_dt * theta_i);
    viscous_strains[i] = (_springs_elasticity_tensors[qp][i].invSymm() * effective_stress) /
                         (theta_i * (1. + gamma));
    viscous_strains[i] +=
        viscous_strains_old[i] * (gamma / (theta_i * (1. + gamma)) - (1. - theta_i) / theta_i);
  }

  if (hasLongtermDashpot(qp))
  {
    viscous_strains.back() = (_first_elasticity_tensor[qp].invSymm() * effective_stress) *
                             (_dt / _dashpot_viscosities[qp].back());
    viscous_strains.back() += viscous_strains_old.back();
  }

  if (has_driving_eigenstrain)
  {
    RankTwoTensor driving_effective_stress =
        _instantaneous_elasticity_tensor[qp] * driving_eigenstrain;

    for (unsigned int i = 0; i < _springs_elasticity_tensors[qp].size(); ++i)
    {
      double theta_i = computeTheta(_dt, _dashpot_viscosities[qp][i]);
      double gamma = _dashpot_viscosities[qp][i] / (_dt * theta_i);
      viscous_strains[i] +=
          (_springs_elasticity_tensors[qp][i].invSymm() * driving_effective_stress) /
          (theta_i * (1. + gamma));
    }

    if (hasLongtermDashpot(qp))
      viscous_strains.back() +=
          (_first_elasticity_tensor[qp].invSymm() * driving_effective_stress) *
          (_dt / _dashpot_viscosities[qp].back());
  }
}
