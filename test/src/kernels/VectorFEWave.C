/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "VectorFEWave.h"
#include "Function.h"

registerMooseObject("MooseTestApp", VectorFEWave);

template <>
InputParameters
validParams<VectorFEWave>()
{
  InputParameters params = validParams<VectorKernel>();
  params.addParam<FunctionName>("x_forcing_func", 0, "The x forcing function.");
  params.addParam<FunctionName>("y_forcing_func", 0, "The y forcing function.");
  params.addParam<FunctionName>("z_forcing_func", 0, "The z forcing function.");
  return params;
}

VectorFEWave::VectorFEWave(const InputParameters & parameters)
  : VectorKernel(parameters),
    _x_ffn(getFunction("x_forcing_func")),
    _y_ffn(getFunction("y_forcing_func")),
    _z_ffn(getFunction("z_forcing_func"))
{
}

Real
VectorFEWave::computeQpResidual()
{
  return _curl_test[_i][_qp] * _curl_u[_qp] + _test[_i][_qp] * _u[_qp] -
         RealVectorValue(_x_ffn.value(_t, _q_point[_qp]),
                         _y_ffn.value(_t, _q_point[_qp]),
                         _z_ffn.value(_t, _q_point[_qp])) *
             _test[_i][_qp];
}

Real
VectorFEWave::computeQpJacobian()
{
  return _curl_test[_i][_qp] * _curl_phi[_j][_qp] + _test[_i][_qp] * _phi[_j][_qp];
}
