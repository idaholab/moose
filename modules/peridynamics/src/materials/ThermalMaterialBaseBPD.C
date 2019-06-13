//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalMaterialBaseBPD.h"
#include "MooseVariable.h"
#include "Function.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ThermalMaterialBaseBPD>()
{
  InputParameters params = validParams<MaterialBasePD>();
  params.addClassDescription("Base class for bond-based peridynamic thermal models");

  params.addRequiredParam<NonlinearVariableName>("temperature",
                                                 "Nonlinear variable name for the temperature");
  params.addRequiredParam<MaterialPropertyName>("thermal_conductivity",
                                                "Name of material defining thermal conductivity");

  return params;
}

ThermalMaterialBaseBPD::ThermalMaterialBaseBPD(const InputParameters & parameters)
  : MaterialBasePD(parameters),
    _temp_var(_subproblem.getVariable(_tid, getParam<NonlinearVariableName>("temperature"))),
    _temp(2),
    _bond_heat_flow(declareProperty<Real>("bond_heat_flow")),
    _bond_dQdT(declareProperty<Real>("bond_dQdT")),
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity"))
{
}

void
ThermalMaterialBaseBPD::computeProperties()
{
  MaterialBasePD::computeProperties();

  Real ave_thermal_conductivity = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    ave_thermal_conductivity += _thermal_conductivity[qp] * _JxW[qp] * _coord[qp];
  ave_thermal_conductivity /= _assembly.elemVolume();

  // compute peridynamic micro-conductivity: _Kij
  computePDMicroConductivity(ave_thermal_conductivity);

  computeNodalTemperature();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    computeBondHeatFlow();
}

void
ThermalMaterialBaseBPD::computeBondHeatFlow()
{
  // residual term
  _bond_heat_flow[_qp] = _Kij * (_temp[1] - _temp[0]) / _origin_length * _nv[0] * _nv[1];

  // derivative of the residual term
  _bond_dQdT[_qp] = _Kij / _origin_length * _nv[0] * _nv[1];
}

void
ThermalMaterialBaseBPD::computeNodalTemperature()
{
  _temp[0] = _temp_var.getNodalValue(*_current_elem->node_ptr(0));
  _temp[1] = _temp_var.getNodalValue(*_current_elem->node_ptr(1));
}
