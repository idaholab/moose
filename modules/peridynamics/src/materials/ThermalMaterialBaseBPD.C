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

InputParameters
ThermalMaterialBaseBPD::validParams()
{
  InputParameters params = PeridynamicsMaterialBase::validParams();
  params.addClassDescription("Base class for bond-based peridynamic thermal models");

  params.addRequiredCoupledVar("temperature", "Nonlinear variable name for the temperature");
  params.addRequiredParam<MaterialPropertyName>(
      "thermal_conductivity", "Name of material property defining thermal conductivity");

  return params;
}

ThermalMaterialBaseBPD::ThermalMaterialBaseBPD(const InputParameters & parameters)
  : PeridynamicsMaterialBase(parameters),
    _temp_var(getVar("temperature", 0)),
    _temp(2),
    _bond_heat_flow(declareProperty<Real>("bond_heat_flow")),
    _bond_dQdT(declareProperty<Real>("bond_dQdT")),
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity"))
{
}

void
ThermalMaterialBaseBPD::computeProperties()
{
  setupMeshRelatedData(); // function from base class

  Real ave_thermal_conductivity = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    ave_thermal_conductivity += _thermal_conductivity[qp] * _JxW[qp] * _coord[qp];

  ave_thermal_conductivity /= _assembly.elemVolume();

  // nodal temperature
  _temp[0] = _temp_var->getNodalValue(*_current_elem->node_ptr(0));
  _temp[1] = _temp_var->getNodalValue(*_current_elem->node_ptr(1));

  // compute peridynamic micro-conductivity: _Kij
  computePeridynamicsParams(ave_thermal_conductivity);

  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    // residual term
    _bond_heat_flow[nd] =
        _Kij * (_temp[1] - _temp[0]) / _origin_vec.norm() * _node_vol[0] * _node_vol[1];

    // derivative of the residual term
    _bond_dQdT[nd] = -_Kij / _origin_vec.norm() * _node_vol[0] * _node_vol[1];
  }
}
