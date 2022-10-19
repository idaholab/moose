//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UniqueIDAux.h"

registerMooseObject("MooseTestApp", UniqueIDAux);

InputParameters
UniqueIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  return params;
}

UniqueIDAux::UniqueIDAux(const InputParameters & parameters) : AuxKernel(parameters) {}

UniqueIDAux::~UniqueIDAux() {}

Real
UniqueIDAux::computeValue()
{
#ifdef LIBMESH_ENABLE_UNIQUE_ID
  if (isNodal())
    return _current_node->unique_id();
  else
    return _current_elem->unique_id();
#else
  return 0;
#endif
}
