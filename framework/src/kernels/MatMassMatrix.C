//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatMassMatrix.h"
#include "MaterialProperty.h"
#include "MooseError.h"

registerMooseObject("MooseApp", MatMassMatrix);

InputParameters
MatMassMatrix::validParams()
{
  InputParameters params = MassMatrixBase::validParams();
  params.addClassDescription(
      "Computes a finite element mass matrix give density as a material property");
  params.addRequiredParam<MaterialPropertyName>("density",
                                                "The material property defining the density");
  return params;
}

MatMassMatrix::MatMassMatrix(const InputParameters & parameters)
  : MassMatrixBase(parameters), _density(getMaterialProperty<Real>("density"))
{
}

Real
MatMassMatrix::computeQpJacobian()
{
  return _test[_i][_qp] * _density[_qp] * _phi[_j][_qp];
}
