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

  /**
   * Compute the local contribution to the action of this nodal boundary condition's Jacobian on
   * the Kokkos matrix-free direction vector, accumulating the result into the Kokkos matrix-free
   * action vector.
   */
  virtual void computeJacobianVectorProduct()
  {
    mooseError("computeJacobianVectorProduct() is not implemented for Kokkos nodal boundary "
               "condition type '",
               type(),
               "'.");
  }

  /**
   * Compute the local contribution to the Kokkos matrix-free Jacobian diagonal, accumulating the
   * result into the Kokkos matrix-free diagonal vector.
   */
  virtual void computeJacobianDiagonal()
  {
    mooseError("computeJacobianDiagonal() is not implemented for Kokkos nodal boundary "
               "condition type '",
               type(),
               "'.");
  }

  /**
   * Evaluate this boundary condition's prescribed value on the host, at an arbitrary point and
   * time. This is what lets libMesh's Dirichlet constraint machinery source the value of a
   * degree of freedom it reports beyond a mesh node (e.g. a HIERARCHIC edge or face mode), which
   * the device-side computeValue() dispatch, keyed on boundary nodes, cannot reach.
   * @param p The point to evaluate the value at
   * @param time The time to evaluate the value at
   * @returns The prescribed value
   */
  virtual Real hostValue(const libMesh::Point & /* p */, Real /* time */) const
  {
    mooseError("Boundary condition '",
               name(),
               "' of type '",
               type(),
               "' has no host-evaluable value, so it cannot pin degrees of freedom libMesh's "
               "Dirichlet constraint machinery reports beyond mesh nodes (e.g. HIERARCHIC "
               "edge/face modes).");
  }

  /**
   * Get whether this is a Dirichlet-type boundary condition, i.e. one with a host-evaluable value
   * that can pin degrees of freedom libMesh's constraint machinery reports beyond mesh nodes
   * @returns Whether this is a Dirichlet-type boundary condition
   */
  virtual bool isDirichletBC() const { return false; }
};

} // namespace Moose::Kokkos
