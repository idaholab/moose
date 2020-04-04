//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HardwareIDAux.h"

registerMooseObject("MooseApp", HardwareIDAux);

InputParameters
HardwareIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Creates a field showing the assignment of partitions to physical nodes in the cluster.");
  return params;
}

HardwareIDAux::HardwareIDAux(const InputParameters & parameters)
  : AuxKernel(parameters), _rank_map(_app.rankMap())
{
}

Real
HardwareIDAux::computeValue()
{
  if (isNodal())
    return _rank_map.hardwareID(_current_node->processor_id());
  else
    return _rank_map.hardwareID(_current_elem->processor_id());
}
