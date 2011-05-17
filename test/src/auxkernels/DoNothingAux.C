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

#include "DoNothingAux.h"

template<>
InputParameters validParams<DoNothingAux>()
{
  InputParameters params = validParams<AuxKernel>();

  return params;
}

DoNothingAux::DoNothingAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters)
{
}

DoNothingAux::~DoNothingAux()
{
}

Real
DoNothingAux::computeValue()
{
  return _u[_qp];
}
