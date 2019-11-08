//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatDivergenceBC.h"

registerMooseObject("MooseTestApp", MatDivergenceBC);

InputParameters
MatDivergenceBC::validParams()
{
  InputParameters params = DivergenceBC::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop_name", "The name of the material property");

  return params;
}

MatDivergenceBC::MatDivergenceBC(const InputParameters & parameters)
  : DivergenceBC(parameters), _mat(getMaterialProperty<Real>("prop_name"))
{
}

MatDivergenceBC::~MatDivergenceBC() {}

Real
MatDivergenceBC::computeQpResidual()
{
  return _mat[_qp] * DivergenceBC::computeQpResidual();
}

Real
MatDivergenceBC::computeQpJacobian()
{
  return _mat[_qp] * DivergenceBC::computeQpJacobian();
}
