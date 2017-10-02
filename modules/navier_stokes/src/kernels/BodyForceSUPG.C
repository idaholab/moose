/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "BodyForceSUPG.h"
#include "Function.h"

template <>
InputParameters
validParams<BodyForceSUPG>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription("SUPG contribution from source term.");
  MooseEnum tau_type("opt mod");
  params.addRequiredParam<MooseEnum>(
      "tau_type", tau_type, "The type of stabilization parameter to use.");
  params.addRequiredParam<FunctionName>("function", "The source function");
  return params;
}

BodyForceSUPG::BodyForceSUPG(const InputParameters & parameters)
  : INSBase(parameters),
    _function(getFunction("function")),
    _tau_type(getParam<MooseEnum>("tau_type"))
{
}

Real
BodyForceSUPG::computeQpResidual()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  if (_tau_type == "opt")
    return -tauNodal() * U * _grad_test[_i][_qp] * _function.value(_t, _q_point[_qp]);
  else
    return -tau() * U * _grad_test[_i][_qp] * _function.value(_t, _q_point[_qp]);
}

Real
BodyForceSUPG::computeQpJacobian()
{
  return 0;
}
