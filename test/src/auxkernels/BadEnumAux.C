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

#include "BadEnumAux.h"

template<>
InputParameters validParams<BadEnumAux>()
{
  InputParameters params = validParams<AuxKernel>();

  MooseEnum choice("this that", "this");

  params.addParam<MooseEnum>("choice", choice, "Time to choose.");

  return params;
}

BadEnumAux::BadEnumAux(const InputParameters & parameters) :
    AuxKernel(parameters)
{
  MooseEnum my_choice = parameters.get<MooseEnum>("choice");

  // use a bad string comparison here
  if (my_choice == "the_other")
    mooseError("Should not reach here because 'the_other' was not in the original MooseEnum");
}

BadEnumAux::~BadEnumAux()
{
}

Real
BadEnumAux::computeValue()
{
  return _u[_qp];
}

