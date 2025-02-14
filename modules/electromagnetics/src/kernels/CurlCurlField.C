//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CurlCurlField.h"

registerMooseObject("ElectromagneticsApp", CurlCurlField);
registerMooseObject("ElectromagneticsApp", ADCurlCurlField);

template <bool is_ad>
InputParameters
CurlCurlFieldTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernelCurl<is_ad>::validParams();
  params.addClassDescription("Weak form term corresponding to $\\nabla \\times (a \\nabla \\times "
                             "\\vec{E})$.");
  params.addParam<Real>("coeff", 1.0, "Weak form coefficient (default = 1.0).");
  return params;
}

template <bool is_ad>
CurlCurlFieldTempl<is_ad>::CurlCurlFieldTempl(const InputParameters & parameters)
  : GenericKernelCurl<is_ad>(parameters), _coeff(this->template getParam<Real>("coeff"))
{
}

template <bool is_ad>
GenericReal<is_ad>
CurlCurlFieldTempl<is_ad>::computeQpResidual()
{
  return _coeff * _curl_u[_qp] * _curl_test[_i][_qp];
}

template <bool is_ad>
Real
CurlCurlFieldTempl<is_ad>::computeQpJacobian()
{
  return _coeff * _curl_phi[_j][_qp] * _curl_test[_i][_qp];
}
