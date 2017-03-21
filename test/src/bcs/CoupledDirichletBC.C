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

#include "CoupledDirichletBC.h"

template <>
InputParameters
validParams<CoupledDirichletBC>()
{
  InputParameters params = validParams<DirichletBC>();
  params.addRequiredCoupledVar("v", "The coupled variable");
  return params;
}

CoupledDirichletBC::CoupledDirichletBC(const InputParameters & parameters)
  : DirichletBC(parameters), _v(coupledValue("v")), _v_num(coupled("v")), _c(1.0)
{
}

Real
CoupledDirichletBC::computeQpResidual()
{
  return _c * _u[_qp] + _u[_qp] * _u[_qp] + _v[_qp] * _v[_qp] - _value;
}

Real
CoupledDirichletBC::computeQpJacobian()
{
  return _c + 2. * _u[_qp];
}

Real
CoupledDirichletBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_num)
    return 2. * _v[_qp];
  else
    return 0.;
}
