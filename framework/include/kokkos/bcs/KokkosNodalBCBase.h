//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosBoundaryCondition.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos nodal boundary conditions
 */
class NodalBCBase : public BoundaryCondition,
                    public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   * @param field_type The MOOSE variable field type
   */
  NodalBCBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  /**
   * Copy constructor for parallel dispatch
   */
  NodalBCBase(const NodalBCBase & object);

  /**
   * Get the list of contiguous node IDs this object is operating on
   * @returns The list of contiguous node IDs
   */
  std::vector<ContiguousNodeID> getContiguousNodes() const;

  /**
   * For use in Dirichlet boundary conditions only
   */
  ///@{
  virtual bool preset() const { return false; }
  virtual void presetSolution(TagID /* tag */) {}
  ///@}
};

} // namespace Moose::Kokkos
