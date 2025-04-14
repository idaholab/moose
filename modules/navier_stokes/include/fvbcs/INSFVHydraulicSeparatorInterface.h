//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVBoundaryCondition.h"
#include "INSFVVariable.h"

/**
 * A base class which serves as a tag for hydraulic separators.
 */
class INSFVHydraulicSeparatorInterface
{
public:
  /// Constructor
  INSFVHydraulicSeparatorInterface(const FVBoundaryCondition * bc);

protected:
  /// Pointer to the boundary condition that inherits from this interface
  const FVBoundaryCondition * _bc;

  /// Pointer to the INSFV variable
  const INSFVVariable * _insfv_variable;
};
