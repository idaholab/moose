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

#include "MultipleUpdateAux.h"

template<>
InputParameters validParams<MultipleUpdateAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("u", "unknown (nl-variable)");
  params.addRequiredCoupledVar("var1", "an aux variable to update");
  params.addRequiredCoupledVar("var2", "another aux variable to update");

  return params;
}

MultipleUpdateAux::MultipleUpdateAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _nl_u(coupledValue("u")),
    _var1(coupledValue("var1")),
    _var2(coupledValue("var2"))
{
}

MultipleUpdateAux::~MultipleUpdateAux()
{
}

Real
MultipleUpdateAux::computeValue()
{
  _var1[_qp] = _nl_u[_qp] + 10.0;
  _var2[_qp] = _nl_u[_qp] + 200.0;
  return -3.33;
}
