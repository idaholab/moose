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
#include "MTICMult.h"

template <>
InputParameters
validParams<MTICMult>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredCoupledVar("var1", "Coupled variable");
  params.addRequiredParam<Real>("factor", "Some factor");

  return params;
}

MTICMult::MTICMult(const InputParameters & parameters)
  : InitialCondition(parameters), _var1(coupledValue("var1")), _factor(getParam<Real>("factor"))
{
}

MTICMult::~MTICMult() {}

Real
MTICMult::value(const Point & /*p*/)
{
  return _var1[_qp] * _factor;
}
