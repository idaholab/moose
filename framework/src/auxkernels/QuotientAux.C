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

#include "QuotientAux.h"

template<>
InputParameters validParams<QuotientAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addCoupledVar("numerator", "The upstairs of the quotient variable");
  params.addCoupledVar("denominator", "The downstairs of the quotient variable");
  return params;
}



QuotientAux::QuotientAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
   _numerator(coupledValue("numerator")),
   _denominator(coupledValue("denominator"))
{}



Real
QuotientAux::computeValue()
{
  return _numerator[_qp] / _denominator[_qp];
}
