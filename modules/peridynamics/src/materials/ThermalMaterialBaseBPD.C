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

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ThermalMaterialBaseBPD>()
{
  InputParameters params = validParams<MaterialBasePD>();
  params.addClassDescription("Base class for bond-based peridynamic thermal models");

  params.addRequiredParam<NonlinearVariableName>("temperature",
                                                 "Nonlinear variable name for the temperature");
  params.addParam<Real>("thermal_conductivity", 0.0, "Value of material thermal conductivity");
  params.addParam<FunctionName>(
      "thermal_conductivity_function", "", "Thermal conductivity as a function of temperature");

  return params;
}

ThermalMaterialBaseBPD::ThermalMaterialBaseBPD(const InputParameters & parameters)
  : MaterialBasePD(parameters),
    _temp_var(_subproblem.getVariable(_tid, getParam<NonlinearVariableName>("temperature"))),
    _temp(2),
    _thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _thermal_conductivity_function(getParam<FunctionName>("thermal_conductivity_function") != ""
                                       ? &getFunction("thermal_conductivity_function")
                                       : NULL),
    _bond_heat_flow(declareProperty<Real>("bond_heat_flow")),
    _bond_dQdT(declareProperty<Real>("bond_dQdT"))
{
  if (_thermal_conductivity_function)
  {
    Point p;
    _kappa = _thermal_conductivity_function->value((_temp[0] + _temp[1]) / 2.0, p);
  }
  else if (_thermal_conductivity)
    _kappa = _thermal_conductivity;
  else
    mooseError("Neither thermal_conductivity nor thermal_conductivity_function is provided for "
               "peridynamic thermal models: ThermalMaterialBaseBPD");
}

void
ThermalMaterialBaseBPD::computeProperties()
{
  MaterialBasePD::computeProperties();

  // compute peridynamic micro-conductivity: _Kij
  computePDMicroConductivity();

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
  _temp[0] = _temp_var.getNodalValue(*_current_elem->get_node(0));
  _temp[1] = _temp_var.getNodalValue(*_current_elem->get_node(1));
}
