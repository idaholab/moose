/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ChemicalOutFlowBC.h"

template <>
InputParameters
validParams<ChemicalOutFlowBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

ChemicalOutFlowBC::ChemicalOutFlowBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _diff(getMaterialProperty<Real>("diffusivity")),
    _porosity(getMaterialProperty<Real>("porosity"))
{
}

Real
ChemicalOutFlowBC::computeQpResidual()
{
  return -_test[_i][_qp] * _porosity[_qp] * _diff[_qp] * _grad_u[_qp] * _normals[_qp];
}

Real
ChemicalOutFlowBC::computeQpJacobian()
{
  return -_test[_i][_qp] * _porosity[_qp] * _diff[_qp] * _grad_phi[_j][_qp] * _normals[_qp];
}
