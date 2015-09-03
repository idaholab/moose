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
#include "ADCoupledValueTest.h"

template<>
InputParameters validParams<ADCoupledValueTest>()
{
  InputParameters params = validParams<ADKernel>();
  params.addCoupledVar("v", 2.0, "The coupled variable.");
  return params;
}


ADCoupledValueTest::ADCoupledValueTest(const InputParameters & parameters) :
    ADKernel(parameters),
    _v(adCoupledValue("v"))
{
}

ADReal
ADCoupledValueTest::computeQpResidual()
{
  return -_v[_qp] * _test[_i][_qp];
}
