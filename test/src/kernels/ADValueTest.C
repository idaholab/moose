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
#include "ADValueTest.h"

template<>
InputParameters validParams<ADValueTest>()
{
  return validParams<ADKernel>();
}


ADValueTest::ADValueTest(const InputParameters & parameters) :
    ADKernel(parameters)
{
}

ADReal
ADValueTest::computeQpResidual()
{
  return -_u[_qp] * _test[_i][_qp];
}
