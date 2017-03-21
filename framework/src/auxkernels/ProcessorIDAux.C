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

#include "ProcessorIDAux.h"

template <>
InputParameters
validParams<ProcessorIDAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

ProcessorIDAux::ProcessorIDAux(const InputParameters & parameters) : AuxKernel(parameters) {}

Real
ProcessorIDAux::computeValue()
{
  if (isNodal())
    return _current_node->processor_id();
  else
    return _current_elem->processor_id();
}
