/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMassBodyForceMMS.h"
#include "Function.h"

template <>
InputParameters
validParams<INSMassBodyForceMMS>()
{
  InputParameters params = validParams<INSBase>();
  params.addClassDescription(
      "Used during MMS testing to consistently include forcing function in PSPG formulation.");
  params.addRequiredParam<FunctionName>("x_vel_forcing_func",
                                        "The x-velocity mms forcing function.");
  params.addParam<FunctionName>("y_vel_forcing_func", 0, "The y-velocity mms forcing function.");
  params.addParam<FunctionName>("z_vel_forcing_func", 0, "The z-velocity mms forcing function.");
  return params;
}

INSMassBodyForceMMS::INSMassBodyForceMMS(const InputParameters & parameters)
  : INSBase(parameters),
    _x_ffn(getFunction("x_vel_forcing_func")),
    _y_ffn(getFunction("y_vel_forcing_func")),
    _z_ffn(getFunction("z_vel_forcing_func"))
{
}

Real
INSMassBodyForceMMS::computeQpResidual()
{
  return -tau() * _grad_test[_i][_qp] * RealVectorValue(-_x_ffn.value(_t, _q_point[_qp]),
                                                        -_y_ffn.value(_t, _q_point[_qp]),
                                                        -_z_ffn.value(_t, _q_point[_qp]));
}

Real
INSMassBodyForceMMS::computeQpJacobian()
{
  return 0;
}

Real
INSMassBodyForceMMS::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
    return -dTauDUComp(0) * _grad_test[_i][_qp] * RealVectorValue(-_x_ffn.value(_t, _q_point[_qp]),
                                                                  -_y_ffn.value(_t, _q_point[_qp]),
                                                                  -_z_ffn.value(_t, _q_point[_qp]));

  else if (jvar == _v_vel_var_number)
    return -dTauDUComp(1) * _grad_test[_i][_qp] * RealVectorValue(-_x_ffn.value(_t, _q_point[_qp]),
                                                                  -_y_ffn.value(_t, _q_point[_qp]),
                                                                  -_z_ffn.value(_t, _q_point[_qp]));

  else if (jvar == _w_vel_var_number)
    return -dTauDUComp(2) * _grad_test[_i][_qp] * RealVectorValue(-_x_ffn.value(_t, _q_point[_qp]),
                                                                  -_y_ffn.value(_t, _q_point[_qp]),
                                                                  -_z_ffn.value(_t, _q_point[_qp]));

  else
    return 0.0;
}
