/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Advection.h"
#include "Function.h"

template <>
InputParameters
validParams<Advection>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription("This class solves the scalar advection equation, "
                             "$\\vec{a}\\cdot\\nabla u = f$ with SUPG stabilization.");
  params.addParam<FunctionName>("forcing_func", 0, "The forcing function, typically used for MMS.");
  MooseEnum tau_type("opt mod");
  params.addRequiredParam<MooseEnum>(
      "tau_type", tau_type, "The type of stabilization parameter to use.");
  return params;
}

Advection::Advection(const InputParameters & parameters)
  : INSBase(parameters),
    _ffn(getFunction("forcing_func")),
    _tau_type(getParam<MooseEnum>("tau_type"))
{
}

Real
Advection::computeQpResidual()
{
  Real tau_val = (_tau_type == "opt" ? tauNodal() : tau());
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return (_test[_i][_qp] + tau_val * (U * _grad_test[_i][_qp])) *
         (U * _grad_u[_qp] - _ffn.value(_t, _q_point[_qp]));
}

Real
Advection::computeQpJacobian()
{
  Real tau_val = (_tau_type == "opt" ? tauNodal() : tau());
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return (_test[_i][_qp] + tau_val * (U * _grad_test[_i][_qp])) * (U * _grad_phi[_j][_qp]);
}
