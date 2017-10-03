/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AdvectionSUPG.h"

template <>
InputParameters
validParams<AdvectionSUPG>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription("SUPG contribution from advection term.");
  MooseEnum tau_type("opt mod");
  params.addRequiredParam<MooseEnum>(
      "tau_type", tau_type, "The type of stabilization parameter to use.");
  return params;
}

AdvectionSUPG::AdvectionSUPG(const InputParameters & parameters)
  : INSBase(parameters), _tau_type(getParam<MooseEnum>("tau_type"))
{
}

Real
AdvectionSUPG::computeQpResidual()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  if (_tau_type == "opt")
    return tauNodal() * U * _grad_test[_i][_qp] * U * _grad_u[_qp];
  else
    return tau() * U * _grad_test[_i][_qp] * U * _grad_u[_qp];
}

Real
AdvectionSUPG::computeQpJacobian()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  if (_tau_type == "opt")
    return tauNodal() * U * _grad_test[_i][_qp] * U * _grad_phi[_j][_qp];
  else
    return tau() * U * _grad_test[_i][_qp] * U * _grad_phi[_j][_qp];
}
