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

InputParameters
PorousFlowMatrixInternalEnergy::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addRequiredParam<Real>("specific_heat_capacity",
                                "Specific heat capacity of the rock grains (J/kg/K).");
  params.addRequiredParam<Real>("density", "Density of the rock grains");
  params.set<bool>("at_nodes") = true;
  params.addPrivateParam<std::string>("pf_material_type", "matrix_internal_energy");
  params.addClassDescription("This Material calculates the internal energy of solid rock grains, "
                             "which is specific_heat_capacity * density * temperature.  Kernels "
                             "multiply this by (1 - porosity) to find the energy density of the "
                             "porous rock in a rock-fluid system");
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
