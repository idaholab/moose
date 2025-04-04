//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVHydraulicSeparatorInterface.h"

INSFVHydraulicSeparatorInterface::INSFVHydraulicSeparatorInterface(const FVBoundaryCondition * bc)
  : _bc(bc)
{
  if (!dynamic_cast<const INSFVVariable *>(&_bc->variable()))
    _bc->paramError("variable", "The variable should be an INSFV variable!");
}
