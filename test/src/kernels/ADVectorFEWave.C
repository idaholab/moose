//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorFEWave.h"
#include "Function.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", ADVectorFEWave);

InputParameters
ADVectorFEWave::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addParam<FunctionName>("x_forcing_func", 0, "The x forcing function.");
  params.addParam<FunctionName>("y_forcing_func", 0, "The y forcing function.");
  params.addParam<FunctionName>("z_forcing_func", 0, "The z forcing function.");
  params.addClassDescription(
      "Weak form term corresponding to $\\nabla \\times (a \\nabla \\times "
      "\\vec{E})$ and the corresponding forcing function using automatic differentiation.");
  return params;
}

ADVectorFEWave::ADVectorFEWave(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _curl_test(_var.curlPhi()),
    _curl_u(_var.adCurlSln()),
    _x_ffn(getFunction("x_forcing_func")),
    _y_ffn(getFunction("y_forcing_func")),
    _z_ffn(getFunction("z_forcing_func"))
{
}

ADReal
ADVectorFEWave::computeQpResidual()
{
  return _curl_test[_i][_qp] * _curl_u[_qp] + _test[_i][_qp] * _u[_qp] -
         RealVectorValue(_x_ffn.value(_t, _q_point[_qp]),
                         _y_ffn.value(_t, _q_point[_qp]),
                         _z_ffn.value(_t, _q_point[_qp])) *
             _test[_i][_qp];
}
