//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CurlCurlField.h"
#include "Assembly.h"

registerMooseObject("ElectromagneticsApp", CurlCurlField);

InputParameters
CurlCurlField::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription("Weak form term corresponding to $\\nabla \\times (a \\nabla \\times "
                             "\\vec{E})$.");
  params.addParam<Real>("coeff", 1.0, "Weak form coefficient (default = 1.0).");
  return params;
}

CurlCurlField::CurlCurlField(const InputParameters & parameters)
  : VectorKernel(parameters),
    _curl_test(_var.curlPhi()),
    _curl_phi(_assembly.curlPhi(_var)),
    _curl_u(_is_implicit ? _var.curlSln() : _var.curlSlnOld()),
    _coeff(getParam<Real>("coeff"))
{
}

Real
CurlCurlField::computeQpResidual()
{
  return _coeff * _curl_u[_qp] * _curl_test[_i][_qp];
}

Real
CurlCurlField::computeQpJacobian()
{
  return _coeff * _curl_phi[_j][_qp] * _curl_test[_i][_qp];
}
