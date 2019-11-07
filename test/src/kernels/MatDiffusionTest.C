//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatDiffusionTest.h"

registerMooseObject("MooseTestApp", MatDiffusionTest);

InputParameters
MatDiffusionTest::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");

  MooseEnum prop_state("current old older", "current");
  params.addParam<MooseEnum>(
      "prop_state", prop_state, "Declares which property state we should retrieve");
  return params;
}

MatDiffusionTest::MatDiffusionTest(const InputParameters & parameters) : Kernel(parameters)
{
  MooseEnum prop_state = getParam<MooseEnum>("prop_state");

  if (prop_state == "current")
    _diff = &getMaterialProperty<Real>("prop_name");
  else if (prop_state == "old")
    _diff = &getMaterialPropertyOld<Real>("prop_name");
  else if (prop_state == "older")
    _diff = &getMaterialPropertyOlder<Real>("prop_name");
}

Real
MatDiffusionTest::computeQpResidual()
{
  return (*_diff)[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
MatDiffusionTest::computeQpJacobian()
{
  return (*_diff)[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
