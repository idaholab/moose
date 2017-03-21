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

#include "ExplicitMidpoint.h"

template <>
InputParameters
validParams<ExplicitMidpoint>()
{
  InputParameters params = validParams<ExplicitRK2>();

  return params;
}

ExplicitMidpoint::ExplicitMidpoint(const InputParameters & parameters) : ExplicitRK2(parameters) {}
