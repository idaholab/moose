//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMatrixInternalEnergy.h"

registerMooseObject("PorousFlowApp", PorousFlowMatrixInternalEnergy);
registerMooseObject("PorousFlowApp", ADPorousFlowMatrixInternalEnergy);

template <bool is_ad>
InputParameters
PorousFlowMatrixInternalEnergyTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addRequiredParam<Real>("specific_heat_capacity",
                                "Specific heat capacity of the rock grains (J/kg/K).");
  params.addRequiredParam<Real>("density", "Density of the rock grains");
  params.set<bool>("at_nodes") = is_ad ? false : true;
  params.addPrivateParam<std::string>("pf_material_type", "matrix_internal_energy");
  params.addClassDescription("This Material calculates the internal energy of solid rock grains, "
                             "which is specific_heat_capacity * density * temperature.  Kernels "
                             "multiply this by (1 - porosity) to find the energy density of the "
                             "porous rock in a rock-fluid system");
  return params;
}

template <bool is_ad>
PorousFlowMatrixInternalEnergyTempl<is_ad>::PorousFlowMatrixInternalEnergyTempl(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _cp(getParam<Real>("specific_heat_capacity")),
    _density(getParam<Real>("density")),
    _heat_cap(_cp * _density),
    _temperature(is_ad ? getGenericMaterialProperty<Real, is_ad>("PorousFlow_temperature_qp")
                       : getGenericMaterialProperty<Real, is_ad>("PorousFlow_temperature_nodal")),
    _dtemperature_dvar(
        is_ad ? nullptr
              : &getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")),
    _energy(declareGenericProperty<Real, is_ad>("PorousFlow_matrix_internal_energy_nodal")),
    _denergy_dvar(is_ad ? nullptr
                        : &declareProperty<std::vector<Real>>(
                              "dPorousFlow_matrix_internal_energy_nodal_dvar"))
{
  if (!is_ad && _nodal_material != true)
    mooseError("PorousFlowMatrixInternalEnergy classes are only defined for at_nodes = true");
}

template <bool is_ad>
void
PorousFlowMatrixInternalEnergyTempl<is_ad>::initQpStatefulProperties()
{
  _energy[_qp] = _heat_cap * _temperature[_qp];
}

template <bool is_ad>
void
PorousFlowMatrixInternalEnergyTempl<is_ad>::computeQpProperties()
{
  _energy[_qp] = _heat_cap * _temperature[_qp];

  if constexpr (!is_ad)
  {
    (*_denergy_dvar)[_qp].assign(_num_var, 0.0);
    for (unsigned v = 0; v < _num_var; ++v)
      (*_denergy_dvar)[_qp][v] = _heat_cap * (*_dtemperature_dvar)[_qp][v];
  }
}

template class PorousFlowMatrixInternalEnergyTempl<false>;
template class PorousFlowMatrixInternalEnergyTempl<true>;
