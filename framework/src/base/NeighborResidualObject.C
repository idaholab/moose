//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NeighborResidualObject.h"

InputParameters
NeighborResidualObject::validParams()
{
  return ResidualObject::validParams();
}

NeighborResidualObject::NeighborResidualObject(const InputParameters & parameters)
  : ResidualObject(parameters)
{
}

void
NeighborResidualObject::prepareNeighborShapes(const unsigned int var_num)
{
  _subproblem.prepareNeighborShapes(var_num, _tid);
}
