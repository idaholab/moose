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

#include "CoupledTimeDerivative.h"

template <>
InputParameters
validParams<CoupledTimeDerivative>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Time derivative Kernel that acts on a coupled variable");
  params.addRequiredCoupledVar("v", "Coupled variable");
  return params;
}

CoupledTimeDerivative::CoupledTimeDerivative(const InputParameters & parameters)
  : Kernel(parameters), _v_dot(coupledDot("v")), _dv_dot(coupledDotDu("v")), _v_var(coupled("v"))
{
}

Real
CoupledTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _v_dot[_qp];
}

Real
CoupledTimeDerivative::computeQpJacobian()
{
  return 0.0;
}

Real
CoupledTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _test[_i][_qp] * _phi[_j][_qp] * _dv_dot[_qp];

  return 0.0;
}
