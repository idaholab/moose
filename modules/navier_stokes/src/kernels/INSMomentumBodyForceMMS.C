/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumBodyForceMMS.h"
#include "Function.h"

template <>
InputParameters
validParams<INSMomentumBodyForceMMS>()
{
  InputParameters params = validParams<INSBase>();
  params.addClassDescription(
      "Used during MMS testing to consistently include forcing function in SUPG formulation.");
  params.addRequiredParam<unsigned>("component", "The velocity component that this is applied to.");
  params.addRequiredParam<FunctionName>("forcing_func", "The mms forcing function.");
  return params;
}

INSMomentumBodyForceMMS::INSMomentumBodyForceMMS(const InputParameters & parameters)
  : INSBase(parameters),
    _component(getParam<unsigned>("component")),
    _ffn(getFunction("forcing_func"))
{
}

Real
INSMomentumBodyForceMMS::computeQpResidual()
{
  return -tau() * RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) * _grad_test[_i][_qp] *
         _ffn.value(_t, _q_point[_qp]);
}

Real
INSMomentumBodyForceMMS::computeQpJacobian()
{
  RealVectorValue d_U_d_U_comp(0, 0, 0);
  d_U_d_U_comp(_component) = _phi[_j][_qp];
  return -_grad_test[_i][_qp] * _ffn.value(_t, _q_point[_qp]) *
         (dTauDUComp(_component) * RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) +
          tau() * d_U_d_U_comp);
}

Real
INSMomentumBodyForceMMS::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
    return -_grad_test[_i][_qp] * _ffn.value(_t, _q_point[_qp]) *
           (dTauDUComp(0) * RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) +
            tau() * RealVectorValue(_phi[_j][_qp], 0, 0));

  else if (jvar == _v_vel_var_number)
    return -_grad_test[_i][_qp] * _ffn.value(_t, _q_point[_qp]) *
           (dTauDUComp(1) * RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) +
            tau() * RealVectorValue(0, _phi[_j][_qp], 0));

  else if (jvar == _w_vel_var_number)
    return -_grad_test[_i][_qp] * _ffn.value(_t, _q_point[_qp]) *
           (dTauDUComp(2) * RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) +
            tau() * RealVectorValue(0, 0, _phi[_j][_qp]));

  else
    return 0.0;
}
