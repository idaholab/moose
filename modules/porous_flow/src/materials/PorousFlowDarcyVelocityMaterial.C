//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDarcyVelocityMaterial.h"

registerMooseObject("PorousFlowApp", PorousFlowDarcyVelocityMaterial);
registerMooseObject("PorousFlowApp", ADPorousFlowDarcyVelocityMaterial);

template <bool is_ad>
InputParameters
PorousFlowDarcyVelocityMaterialTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterial::validParams();
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addClassDescription("This Material calculates the Darcy velocity for all phases");
  params.addPrivateParam<std::string>("pf_material_type", "darcy_velocity");
  params.set<bool>("at_nodes") = false;
  return params;
}

template <bool is_ad>
PorousFlowDarcyVelocityMaterialTempl<is_ad>::PorousFlowDarcyVelocityMaterialTempl(
    const InputParameters & parameters)
  : PorousFlowMaterial(parameters),
    _num_phases(_dictator.numPhases()),
    _num_var(_dictator.numVariables()),
    _permeability(getGenericMaterialProperty<RealTensorValue, is_ad>("PorousFlow_permeability_qp")),
    _dpermeability_dvar(is_ad ? nullptr
                              : &getMaterialProperty<std::vector<RealTensorValue>>(
                                    "dPorousFlow_permeability_qp_dvar")),
    _dpermeability_dgradvar(is_ad ? nullptr
                                  : &getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
                                        "dPorousFlow_permeability_qp_dgradvar")),
    _fluid_density(
        getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_dvar(is_ad ? nullptr
                               : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                     "dPorousFlow_fluid_phase_density_qp_dvar")),
    _fluid_viscosity(
        getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_viscosity_qp")),
    _dfluid_viscosity_dvar(is_ad ? nullptr
                                 : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                       "dPorousFlow_viscosity_qp_dvar")),
    _relative_permeability(getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_relative_permeability_qp")),
    _drelative_permeability_dvar(is_ad ? nullptr
                                       : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                             "dPorousFlow_relative_permeability_qp_dvar")),
    _grad_p(getGenericMaterialProperty<std::vector<RealGradient>, is_ad>(
        "PorousFlow_grad_porepressure_qp")),
    _dgrad_p_dgradvar(is_ad ? nullptr
                            : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                  "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgrad_p_dvar(is_ad ? nullptr
                        : &getMaterialProperty<std::vector<std::vector<RealGradient>>>(
                              "dPorousFlow_grad_porepressure_qp_dvar")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _darcy_velocity(declareGenericProperty<std::vector<RealVectorValue>, is_ad>(
        "PorousFlow_darcy_velocity_qp")),
    _ddarcy_velocity_dvar(is_ad ? nullptr
                                : &declareProperty<std::vector<std::vector<RealVectorValue>>>(
                                      "dPorousFlow_darcy_velocity_qp_dvar")),
    _ddarcy_velocity_dgradvar(
        is_ad ? nullptr
              : &declareProperty<std::vector<std::vector<std::vector<RealVectorValue>>>>(
                    "dPorousFlow_darcy_velocity_qp_dgradvar"))
{
  if (_nodal_material == true)
    mooseError("PorousFlowDarcyVelocityMaterial is only defined for at_nodes = false");
}

template <bool is_ad>
void
PorousFlowDarcyVelocityMaterialTempl<is_ad>::computeQpProperties()
{
  _darcy_velocity[_qp].resize(_num_phases);

  for (unsigned ph = 0; ph < _num_phases; ++ph)
    _darcy_velocity[_qp][ph] =
        -(_permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density[_qp][ph] * _gravity) *
          _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph]);

  if constexpr (!is_ad)
  {
    (*_ddarcy_velocity_dvar)[_qp].resize(_num_phases);
    for (unsigned ph = 0; ph < _num_phases; ++ph)
      (*_ddarcy_velocity_dvar)[_qp][ph].resize(_num_var);

    for (unsigned ph = 0; ph < _num_phases; ++ph)
      for (unsigned v = 0; v < _num_var; ++v)
      {
        (*_ddarcy_velocity_dvar)[_qp][ph][v] =
            -(*_dpermeability_dvar)[_qp][v] *
            (_grad_p[_qp][ph] - _fluid_density[_qp][ph] * _gravity) *
            _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph];
        (*_ddarcy_velocity_dvar)[_qp][ph][v] -=
            _permeability[_qp] *
            ((*_dgrad_p_dvar)[_qp][ph][v] - (*_dfluid_density_dvar)[_qp][ph][v] * _gravity) *
            _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph];
        (*_ddarcy_velocity_dvar)[_qp][ph][v] -=
            _permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density[_qp][ph] * _gravity) *
            ((*_drelative_permeability_dvar)[_qp][ph][v] / _fluid_viscosity[_qp][ph] -
             _relative_permeability[_qp][ph] * (*_dfluid_viscosity_dvar)[_qp][ph][v] /
                 std::pow(_fluid_viscosity[_qp][ph], 2));
      }

    (*_ddarcy_velocity_dgradvar)[_qp].resize(_num_phases);
    for (unsigned ph = 0; ph < _num_phases; ++ph)
    {
      (*_ddarcy_velocity_dgradvar)[_qp][ph].resize(LIBMESH_DIM);
      for (unsigned i = 0; i < LIBMESH_DIM; ++i)
        (*_ddarcy_velocity_dgradvar)[_qp][ph][i].resize(_num_var);
    }

    for (unsigned ph = 0; ph < _num_phases; ++ph)
      for (unsigned i = 0; i < LIBMESH_DIM; ++i)
        for (unsigned v = 0; v < _num_var; ++v)
        {
          (*_ddarcy_velocity_dgradvar)[_qp][ph][i][v] =
              -(*_dpermeability_dgradvar)[_qp][i][v] *
              (_grad_p[_qp][ph] - _fluid_density[_qp][ph] * _gravity);
          for (unsigned j = 0; j < LIBMESH_DIM; ++j)
            (*_ddarcy_velocity_dgradvar)[_qp][ph][i][v](j) -=
                _permeability[_qp](j, i) * (*_dgrad_p_dgradvar)[_qp][ph][v];
          (*_ddarcy_velocity_dgradvar)[_qp][ph][i][v] *=
              _relative_permeability[_qp][ph] / _fluid_viscosity[_qp][ph];
        }
  }
}

template class PorousFlowDarcyVelocityMaterialTempl<false>;
template class PorousFlowDarcyVelocityMaterialTempl<true>;
