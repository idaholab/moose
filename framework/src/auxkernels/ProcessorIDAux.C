//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProcessorIDAux.h"

template <>
InputParameters
validParams<ProcessorIDAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Creates a field showing the processors and partitioning.");
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
