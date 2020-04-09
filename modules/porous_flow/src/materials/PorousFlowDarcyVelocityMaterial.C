//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDarcyVelocityMaterial.h"

registerMooseObject("PorousFlowApp", PorousFlowDarcyVelocityMaterial);

InputParameters
PorousFlowDarcyVelocityMaterial::validParams()
{
  InputParameters params = PorousFlowMaterial::validParams();
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addClassDescription("This Material calculates the Darcy velocity for all phases");
  params.addPrivateParam<std::string>("pf_material_type", "darcy_velocity");
  params.set<bool>("at_nodes") = false;
  return params;
}

PorousFlowDarcyVelocityMaterial::PorousFlowDarcyVelocityMaterial(const InputParameters & parameters)
  : PorousFlowMaterial(parameters),
    _num_phases(_dictator.numPhases()),
    _num_var(_dictator.numVariables()),
    _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_dvar(
        getMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_dgradvar(getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
        "dPorousFlow_permeability_qp_dgradvar")),
    _fluid_density(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_qp_dvar")),
    _fluid_viscosity(getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _dfluid_viscosity_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_qp_dvar")),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_relative_permeability_qp_dvar")),
    _grad_p(getMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _dgrad_p_dgradvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgrad_p_dvar(getMaterialProperty<std::vector<std::vector<RealGradient>>>(
        "dPorousFlow_grad_porepressure_qp_dvar")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _darcy_velocity(declareProperty<std::vector<RealVectorValue>>("PorousFlow_darcy_velocity_qp")),
    _ddarcy_velocity_dvar(declareProperty<std::vector<std::vector<RealVectorValue>>>(
        "dPorousFlow_darcy_velocity_qp_dvar")),
    _ddarcy_velocity_dgradvar(
        declareProperty<std::vector<std::vector<std::vector<RealVectorValue>>>>(
            "dPorousFlow_darcy_velocity_qp_dgradvar"))
{
  if (_nodal_material == true)
    mooseError("PorousFlowDarcyVelocityMaterial is only defined for at_nodes = false");
}

void
PorousFlowDarcyVelocityMaterial::computeQpProperties()
{
  _darcy_velocity[_qp].resize(_num_phases);

  for (unsigned ph = 0; ph < _num_phases; ++ph)
    _darcy_velocity[_qp][ph] =
        -(_permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density[_qp][ph] * _gravity) *
          _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph]);

  _ddarcy_velocity_dvar[_qp].resize(_num_phases);
  for (unsigned ph = 0; ph < _num_phases; ++ph)
    _ddarcy_velocity_dvar[_qp][ph].resize(_num_var);

  for (unsigned ph = 0; ph < _num_phases; ++ph)
    for (unsigned v = 0; v < _num_var; ++v)
    {
      _ddarcy_velocity_dvar[_qp][ph][v] =
          -_dpermeability_dvar[_qp][v] * (_grad_p[_qp][ph] - _fluid_density[_qp][ph] * _gravity) *
          _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph];
      _ddarcy_velocity_dvar[_qp][ph][v] -=
          _permeability[_qp] *
          (_dgrad_p_dvar[_qp][ph][v] - _dfluid_density_dvar[_qp][ph][v] * _gravity) *
          _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph];
      _ddarcy_velocity_dvar[_qp][ph][v] -=
          _permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density[_qp][ph] * _gravity) *
          (_drelative_permeability_dvar[_qp][ph][v] / _fluid_viscosity[_qp][ph] -
           _relative_permeability[_qp][ph] * _dfluid_viscosity_dvar[_qp][ph][v] /
               std::pow(_fluid_viscosity[_qp][ph], 2));
    }

  _ddarcy_velocity_dgradvar[_qp].resize(_num_phases);
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    _ddarcy_velocity_dgradvar[_qp][ph].resize(LIBMESH_DIM);
    for (unsigned i = 0; i < LIBMESH_DIM; ++i)
      _ddarcy_velocity_dgradvar[_qp][ph][i].resize(_num_var);
  }

  for (unsigned ph = 0; ph < _num_phases; ++ph)
    for (unsigned i = 0; i < LIBMESH_DIM; ++i)
      for (unsigned v = 0; v < _num_var; ++v)
      {
        _ddarcy_velocity_dgradvar[_qp][ph][i][v] =
            -_dpermeability_dgradvar[_qp][i][v] *
            (_grad_p[_qp][ph] - _fluid_density[_qp][ph] * _gravity);
        for (unsigned j = 0; j < LIBMESH_DIM; ++j)
          _ddarcy_velocity_dgradvar[_qp][ph][i][v](j) -=
              _permeability[_qp](j, i) * _dgrad_p_dgradvar[_qp][ph][v];
        _ddarcy_velocity_dgradvar[_qp][ph][i][v] *=
            _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph];
      }
}
