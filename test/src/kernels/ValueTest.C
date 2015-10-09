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
#include "ValueTest.h"

template<>
InputParameters validParams<ValueTest>()
{
  return validParams<Kernel>();
}


ValueTest::ValueTest(const InputParameters & parameters) :
    Kernel(parameters)
{
}

Real
ValueTest::computeQpResidual()
{
  return -_u[_qp] * _test[_i][_qp];
}

Real
ValueTest::computeQpJacobian()
{
  return -_phi[_j][_qp] * _test[_i][_qp];
}
