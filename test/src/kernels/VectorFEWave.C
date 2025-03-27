//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorFEWave.h"
#include "Function.h"

registerMooseObject("MooseTestApp", VectorFEWave);
registerMooseObject("MooseTestApp", ADVectorFEWave);

template <bool is_ad>
InputParameters
VectorFEWaveTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernelCurl<is_ad>::validParams();
  params.addParam<FunctionName>("x_forcing_func", 0, "The x forcing function.");
  params.addParam<FunctionName>("y_forcing_func", 0, "The y forcing function.");
  params.addParam<FunctionName>("z_forcing_func", 0, "The z forcing function.");
  return params;
}

template <bool is_ad>
VectorFEWaveTempl<is_ad>::VectorFEWaveTempl(const InputParameters & parameters)
  : GenericKernelCurl<is_ad>(parameters),
    _x_ffn(getFunction("x_forcing_func")),
    _y_ffn(getFunction("y_forcing_func")),
    _z_ffn(getFunction("z_forcing_func"))
{
}

template <bool is_ad>
GenericReal<is_ad>
VectorFEWaveTempl<is_ad>::computeQpResidual()
{
  return _curl_test[_i][_qp] * _curl_u[_qp] + _test[_i][_qp] * _u[_qp] -
         RealVectorValue(_x_ffn.value(_t, _q_point[_qp]),
                         _y_ffn.value(_t, _q_point[_qp]),
                         _z_ffn.value(_t, _q_point[_qp])) *
             _test[_i][_qp];
}

template <bool is_ad>
Real
VectorFEWaveTempl<is_ad>::computeQpJacobian()
{
  return _curl_test[_i][_qp] * _curl_phi[_j][_qp] + _test[_i][_qp] * _phi[_j][_qp];
}
