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

#include "RobinBC.h"

template <>
InputParameters
validParams<RobinBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

RobinBC::RobinBC(const InputParameters & parameters) : IntegratedBC(parameters) {}

Real
RobinBC::computeQpResidual()
{
  return _test[_i][_qp] * 2. * _u[_qp];
  // return _test[_i][_qp] * -2. * _grad_u[_qp] * _normals[_qp];
}

Real
RobinBC::computeQpJacobian()
{
  return _test[_i][_qp] * 2. * _phi[_j][_qp];
  // return _test[_i][_qp] * -2. * _grad_phi[_j][_qp] * _normals[_qp];
}
