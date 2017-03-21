/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SharpInterfaceForcing.h"

template <>
InputParameters
validParams<SharpInterfaceForcing>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<FunctionName>("x_center",
                                        "The parametric x center of the forcing function");
  params.addRequiredParam<FunctionName>("y_center",
                                        "The parametric y center of the forcing function");
  params.addParam<Real>("amplitude", 1.0, "The amplitude of the forcing function");
  return params;
}

SharpInterfaceForcing::SharpInterfaceForcing(const InputParameters & parameters)
  : Kernel(parameters),
    _x_center(getFunction("x_center")),
    _y_center(getFunction("y_center")),
    _amplitude(getParam<Real>("amplitude"))
{
}

Real
SharpInterfaceForcing::computeQpResidual()
{
  Point current_point = _q_point[_qp];
  Real distance =
      (current_point -
       Point(_x_center.value(_t, _q_point[_qp]), _y_center.value(_t, _q_point[_qp]), 0.0))
          .norm();

  if (distance <= 0.1)
    return -_amplitude * _test[_i][_qp];
  else
    return 0.0;
}
