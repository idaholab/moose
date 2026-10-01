//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosSystem.h"

class MooseMesh;
class SystemBase;

namespace Moose::Kokkos
{

class Assembly;
class NodalBCBase;

/**
 * The Kokkos FE system class. Each system in MOOSE with FE variables has a corresponding Kokkos FE
 * system.
 */
class FESystem : public System
{
public:
  /**
   * Constructor for standalone use (pure FE simulations)
   * @param system The associated MOOSE system
   */
  FESystem(SystemBase & system);

  /**
   * Constructor for mixed FE+FV simulations, sharing device memory with an existing System
   * @param system The existing Kokkos System whose device memory to share
   */
  FESystem(System & system);

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Allocate the quadrature point vectors for active variable and tags and cache
   * quadrature point values
   */
  void reinit();

  /**
   * Get the list of off-diagonal coupled field variable numbers of a variable
   * @param var The variable number
   * @returns The list of off-diagonal coupled field variable numbers
   */
  KOKKOS_FUNCTION const Array<unsigned int> & getFieldCoupling(unsigned int var) const
  {
    return _field_coupling[var];
  }

  /**
   * Get the list of off-diagonal coupled scalar variable numbers of a variable
   * @param var The variable number
   * @returns The list of off-diagonal coupled scalar variable numbers
   */
  KOKKOS_FUNCTION const Array<unsigned int> & getScalarCoupling(unsigned int var) const
  {
    return _scalar_coupling[var];
  }

  /**
   * Check whether a local DOF index is associated with a nodal BC for an extra matrix tag
   * @param dof The local DOF index
   * @param tag The extra matrix tag
   * @returns Whether the local DOF index is covered by a nodal BC
   */
  KOKKOS_FUNCTION bool hasNodalBCMatrixTag(dof_id_type dof, TagID tag) const
  {
    return _nbc_matrix_tag_dof[tag].isAlloc() && _nbc_matrix_tag_dof[tag][dof];
  }

  /**
   * Get the FE type ID of a variable
   * @param var The variable number
   * @returns The FE type ID
   */
  KOKKOS_FUNCTION unsigned int getFETypeID(unsigned int var) const { return _var_fe_types[var]; }

  /**
   * Get the local DOF index of a variable for a node
   * @param node The contiguous node ID
   * @param i The node-local DOF index
   * @param var The variable number
   * @returns The local DOF index
   */
  KOKKOS_FUNCTION dof_id_type getNodeLocalDofIndex(ContiguousNodeID node,
                                                   unsigned int i,
                                                   unsigned int var) const
  {
    return _local_node_dof_index[var][node] + i;
  }

  /**
   * Get the global DOF index of a variable for a node
   * @param node The contiguous node ID
   * @param i The node-local DOF index
   * @param var The variable number
   * @returns The global DOF index
   */
  KOKKOS_FUNCTION dof_id_type getNodeGlobalDofIndex(ContiguousNodeID node,
                                                    unsigned int i,
                                                    unsigned int var) const
  {
    return _local_to_global_dof_index[getNodeLocalDofIndex(node, i, var)];
  }

  /**
   * Check whether a variable is vector-valued
   * @param var The variable number
   * @returns Whether the variable is vector-valued
   */
  KOKKOS_FUNCTION bool isVectorVariable(unsigned int var) const { return _var_is_vector[var]; }

  /**
   * Get whether a variable is defined on a node
   * @param node The contiguous node ID
   * @param var The variable number
   * @returns Whether the variable is defined on the node
   */
  KOKKOS_FUNCTION bool isNodalDefined(ContiguousNodeID node, unsigned int var) const
  {
    return _local_node_dof_index[var][node] != libMesh::DofObject::invalid_id;
  }

  /**
   * Get the DOF value of a tagged vector for automatic differentiation (AD)
   * @param dof The local DOF index
   * @param tag The vector tag
   * @param seed The derivative seed
   * @returns The DOF AD value with optional seed derivative
   */
  KOKKOS_FUNCTION ADReal getVectorDofADValue(const dof_id_type dof,
                                             const TagID tag,
                                             const Real seed) const;
#endif

private:
  /**
   * Setup variable data
   */
  void setupVariables();

  /**
   * Setup DOF data
   */
  void setupDofs();

  /**
   * Setup coupling data between variables
   */
  void setupCoupling();

  /**
   * Mark the DOFs covered by nodal BCs
   */
  void setupNodalBCDofs();

  /**
   * Get the list of DOFs covered by a nodal BC
   * @param nbc The Kokkos nodal BC object
   * @param dofs Local-plus-ghost DOF mask; entries are set true for DOFs covered by the nodal BC
   */
  void getNodalBCDofs(const NodalBCBase * nbc, Array<bool> & dofs);

  /**
   * Local nodal DOF indices of each variable
   */
  Array<Array<dof_id_type>> _local_node_dof_index;

  /**
   * FE type ID of each variable
   */
  Array<unsigned int> _var_fe_types;

  /**
   * Whether each variable is vector-valued
   */
  Array<bool> _var_is_vector;

  /**
   * Off-diagonal coupled field and scalar variable numbers of each variable
   */
  ///@{
  Array<Array<unsigned int>> _field_coupling;
  Array<Array<unsigned int>> _scalar_coupling;
  ///@}

  /**
   * Per-matrix-tag local-plus-ghost DOF masks for nodal BC coverage
   */
  Array<Array<bool>> _nbc_matrix_tag_dof;
};

#ifdef MOOSE_KOKKOS_SCOPE
KOKKOS_FUNCTION inline ADReal
FESystem::getVectorDofADValue(const dof_id_type dof, TagID tag, const Real seed) const
{
  ADReal value = _vectors[tag][dof];

  if (seed != 0)
    value.derivatives().insert(_local_to_global_dof_index[dof]) = seed;

  return value;
}
#endif

MakeSystemHolder(FESystem);
} // namespace Moose::Kokkos
