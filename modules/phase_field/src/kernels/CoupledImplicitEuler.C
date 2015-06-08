/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledImplicitEuler.h"

template<>
InputParameters validParams<CoupledImplicitEuler>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Time derivative Kernel that acts on a coupled variable");
  params.addRequiredCoupledVar("v", "Coupled variable");
  return params;
}

CoupledImplicitEuler::CoupledImplicitEuler(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _v_dot(coupledDot("v")),
    _dv_dot(coupledDotDu("v")),
    _v_var(coupled("v"))
{}

Real
CoupledImplicitEuler::computeQpResidual()
{
  return _test[_i][_qp] * _v_dot[_qp];
}

Real
CoupledImplicitEuler::computeQpJacobian()
{
  return 0.0;
}

Real
CoupledImplicitEuler::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _test[_i][_qp] * _phi[_j][_qp] * _dv_dot[_qp];

  return 0.0;
}
