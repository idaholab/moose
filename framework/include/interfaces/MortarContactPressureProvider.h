//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

namespace libMesh
{
class Node;
}

/**
 * Interface for objects that provide the physical normal contact pressure of a mortar interface.
 * A contact Lagrange multiplier variable can store a scaled pressure, so objects in other modules
 * that need the pressure obtain it from the contact object through this interface.
 */
class MortarContactPressureProvider
{
public:
  virtual ~MortarContactPressureProvider() = default;

  /**
   * @param node A secondary node of the mortar interface
   * @return The physical normal contact pressure at \p node, carrying its derivatives with
   * respect to the solution
   */
  virtual ADReal nodalContactPressure(const libMesh::Node & node) const = 0;

  /**
   * @return The basis that interpolates the nodal contact pressures on the current secondary
   * lower-dimensional element, evaluated at the current mortar segment quadrature points
   */
  virtual const VariableTestValue & tractionBasis() const = 0;
};
