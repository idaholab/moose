//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledCurlSuppliedFieldProduct.h"
#include "Function.h"

registerMooseObject("MooseTestApp", ADCoupledCurlSuppliedFieldProduct);

InputParameters
ADCoupledCurlSuppliedFieldProduct::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("vector", "The vector variable for the coupled curl.");
  params.addRequiredParam<FunctionName>("supplied_field_x",
                                        "The x component of the supplied field.");
  params.addRequiredParam<FunctionName>("supplied_field_y",
                                        "The y component of the supplied field.");
  params.addRequiredParam<FunctionName>("supplied_field_z",
                                        "The z component of the supplied field.");
  params.addClassDescription(
      "Supplies the product of the curl of a coupled vector variable and a user "
      "supplied field.");
  return params;
}

ADCoupledCurlSuppliedFieldProduct::ADCoupledCurlSuppliedFieldProduct(
    const InputParameters & parameters)
  : ADKernel(parameters),
    _coupled_curl(adCoupledCurl("vector")),
    _function_x(getFunction("supplied_field_x")),
    _function_y(getFunction("supplied_field_y")),
    _function_z(getFunction("supplied_field_z"))
{
}

ADReal
ADCoupledCurlSuppliedFieldProduct::computeQpResidual()
{
  ADRealVectorValue func_u = {_function_x.value(_t, _q_point[_qp]),
                              _function_y.value(_t, _q_point[_qp]),
                              _function_z.value(_t, _q_point[_qp])};

  return _test[_i][_qp] * _coupled_curl[_qp] * func_u;
}
