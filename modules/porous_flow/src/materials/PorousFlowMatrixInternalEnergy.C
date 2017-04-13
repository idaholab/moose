/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMatrixInternalEnergy.h"

template <>
InputParameters
validParams<PorousFlowMatrixInternalEnergy>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addRequiredParam<Real>("specific_heat_capacity",
                                "Specific heat capacity of the rock grains (J/kg/K).  Internal "
                                "energy = specific_heat_capacity * density * temperature, and this "
                                "is multiplied by (1 - porosity) to find the energy density of the "
                                "rock in a rock-fluid system.");
  params.addRequiredParam<Real>("density", "Density of the rock grains");
  params.set<bool>("at_nodes") = true;
  params.addClassDescription("This Material calculates rock-fluid combined thermal conductivity by "
                             "using a weighted sum.  Thermal conductivity = "
                             "dry_thermal_conductivity + S^exponent * (wet_thermal_conductivity - "
                             "dry_thermal_conductivity), where S is the aqueous saturation");
  return params;
}

PorousFlowMatrixInternalEnergy::PorousFlowMatrixInternalEnergy(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _cp(getParam<Real>("specific_heat_capacity")),
    _density(getParam<Real>("density")),
    _heat_cap(_cp * _density),
    _temperature_nodal(getMaterialProperty<Real>("PorousFlow_temperature_nodal")),
    _dtemperature_nodal_dvar(
        getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")),
    _en_nodal(declareProperty<Real>("PorousFlow_matrix_internal_energy_nodal")),
    _den_nodal_dvar(
        declareProperty<std::vector<Real>>("dPorousFlow_matrix_internal_energy_nodal_dvar"))
{
  if (_nodal_material != true)
    mooseError("PorousFlowMatrixInternalEnergy classes are only defined for at_nodes = true");
}

void
PorousFlowMatrixInternalEnergy::initQpStatefulProperties()
{
  _en_nodal[_qp] = _heat_cap * _temperature_nodal[_qp];
}

void
PorousFlowMatrixInternalEnergy::computeQpProperties()
{
  _en_nodal[_qp] = _heat_cap * _temperature_nodal[_qp];

  _den_nodal_dvar[_qp].assign(_num_var, 0.0);
  for (unsigned v = 0; v < _num_var; ++v)
    _den_nodal_dvar[_qp][v] = _heat_cap * _dtemperature_nodal_dvar[_qp][v];
}
