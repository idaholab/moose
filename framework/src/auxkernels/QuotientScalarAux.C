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

#include "QuotientScalarAux.h"

template <>
InputParameters
validParams<QuotientScalarAux>()
{
  InputParameters params = validParams<AuxScalarKernel>();
  params.addCoupledVar("numerator", "The upstairs of the quotient variable");
  params.addCoupledVar("denominator", "The downstairs of the quotient variable");

  return params;
}

QuotientScalarAux::QuotientScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _a(coupledScalarValue("numerator")),
    _b(coupledScalarValue("denominator"))
{
}

Real
QuotientScalarAux::computeValue()
{
  return _a[0] / _b[0];
}
