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

#include "LinearCombinationAux.h"

template<>
InputParameters validParams<LinearCombinationAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("v", "Vector of 'first' variables.");
  params.addRequiredCoupledVar("w", "Vector of 'second' variables.");

  return params;
}

LinearCombinationAux::LinearCombinationAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _n(coupledComponents("v"))
{
  if (_n != coupledComponents("w"))
    mooseError(name << ": the length of v and w has to be the same");

  _v.resize(_n);
  _w.resize(_n);
  for (unsigned int i = 0; i < _n; i++)
  {
    _v[i] = &coupledValue("v", i);
    _w[i] = &coupledValue("w", i);
  }
}

LinearCombinationAux::~LinearCombinationAux()
{
}

Real
LinearCombinationAux::computeValue()
{
  Real u = 0;
  for (unsigned int i = 0; i < _n; i++)
    u += (*_v[i])[_qp] * (*_w[i])[_qp];
  return u;
}
