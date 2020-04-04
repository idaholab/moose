//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChemicalOutFlowBC.h"

registerMooseObject("ChemicalReactionsApp", ChemicalOutFlowBC);

InputParameters
ChemicalOutFlowBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Chemical flux boundary condition");
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
