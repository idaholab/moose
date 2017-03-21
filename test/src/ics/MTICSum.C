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
#include "MTICSum.h"

template <>
InputParameters
validParams<MTICSum>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredCoupledVar("var1", "First variable");
  params.addRequiredCoupledVar("var2", "Second variable");

  return params;
}

MTICSum::MTICSum(const InputParameters & parameters)
  : InitialCondition(parameters), _var1(coupledValue("var1")), _var2(coupledValue("var2"))
{
}

MTICSum::~MTICSum() {}

Real
MTICSum::value(const Point & /*p*/)
{
  return _var1[_qp] + _var2[_qp] + 3;
}
