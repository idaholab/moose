//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUResidualObject.h"

#include "BoundaryRestrictableRequired.h"

#pragma once

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for Kokkos boundary conditions
 */
class BoundaryCondition : public ResidualObject, public BoundaryRestrictableRequired
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   * @param field_type The MOOSE variable field type
   * @param nodal Whether the boundary condition is a nodal boundary condition
   */
  BoundaryCondition(const InputParameters & parameters, Moose::VarFieldType field_type, bool nodal);
  /**
   * Copy constructor for parallel dispatch
   */
  BoundaryCondition(const BoundaryCondition & object);
};

} // namespace Kokkos
} // namespace Moose

#define usingKokkosBoundaryConditionMembers                                                        \
  usingKokkosResidualObjectMembers;                                                                \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::BoundaryCondition::boundaryElementSideID;                                   \
  using Moose::Kokkos::BoundaryCondition::boundaryNodeID
