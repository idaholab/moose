//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCurlCurlField.h"
#include "Assembly.h"

registerMooseObject("ElectromagneticsApp", ADCurlCurlField);

InputParameters
ADCurlCurlField::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription("Weak form term corresponding to $\\nabla \\times (a \\nabla \\times "
                             "\\vec{E})$ using automatic differentiation.");
  params.addParam<Real>("coeff", 1.0, "Weak form coefficient (default = 1.0).");
  return params;
}

ADCurlCurlField::ADCurlCurlField(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _curl_test(_var.curlPhi()),
    _curl_u(_var.adCurlSln()),
    _coeff(getParam<Real>("coeff"))
{
}

ADReal
ADCurlCurlField::computeQpResidual()
{
  return _coeff * _curl_u[_qp] * _curl_test[_i][_qp];
}
