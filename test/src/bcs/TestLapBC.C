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

#include "TestLapBC.h"

template <>
InputParameters
validParams<TestLapBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

TestLapBC::TestLapBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _second_u(second()),
    _second_phi(secondPhiFace()),
    _second_test(secondTestFace())
{
}

Real
TestLapBC::computeQpResidual()
{
  return -0.5 * _second_u[_qp].tr() * _test[_i][_qp];
}

Real
TestLapBC::computeQpJacobian()
{
  return -0.5 * _second_phi[_j][_qp].tr() * _test[_i][_qp];
}
