//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"
#include "KokkosVector.h"
#include "KokkosMatrix.h"
#include "PerfGraphInterface.h"
#include "KokkosMesh.h"

#include "libmesh/communicator.h"

class MooseMesh;
class SystemBase;

namespace Moose::Kokkos
{

/**
 * The Kokkos base system class. Each system in MOOSE has a corresponding Kokkos
 * base system.
 */
class System : public PerfGraphInterface, public MeshHolder
{
public:
  /**
   * Constructor
   * @param system The associated MOOSE system
   */
  System(SystemBase & system);

  /**
   * Copy constructor - shallow-copies all Array<> members so that device memory is shared.
   * Used by FESystem in mixed FE+FV simulations.
   */
  System(const System & src);

  /**
   * CSR format sparsity data
   */
  struct Sparsity
  {
    Array<PetscInt> col_idx;
    Array<PetscInt> row_idx;
    Array<PetscInt> row_ptr;
  };

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Synchronize the active tagged vectors and matrices between host and device
   * @param dir Copy direction
   */
  void sync(const MemcpyType dir);
  /**
   * Synchronize the specified tagged vectors between host and device
   * @param tags The vector tags
   * @param dir Copy direction
   */
  ///@{
  void sync(const std::set<TagID> & tags, const MemcpyType dir);
  void sync(const std::vector<TagID> & tags, const MemcpyType dir);
  void sync(const TagID tag, const MemcpyType dir);
  ///@}

  /**
   * Set the active variables
   * @param vars The active MOOSE variables
   */
  void setActiveVariables(const std::set<MooseVariableFieldBase *> & vars);

  /**
   * Set the active solution tags
   * @param tags The active solution tags
   */
  void setActiveSolutionTags(const std::set<TagID> & tags);

  /**
   * Set the active residual tags
   * @param tags The active residual tags
   */
  void setActiveResidualTags(const std::set<TagID> & tags);

  /**
   * Set the active matrix tags
   * @param vars The active matrix tags
   */
  void setActiveMatrixTags(const std::set<TagID> & tags);

  /**
   * Clear the cached active variables
   */
  void clearActiveVariables() { _active_variables.destroy(); }

  /**
   * Clear the cached active solution tags
   */
  void clearActiveSolutionTags() { _active_solution_tags.destroy(); }

  /**
   * Clear the cached active residual tags
   */
  void clearActiveResidualTags()
  {
    _active_residual_tags.destroy();
    _residual_tag_active = false;
  }

  /**
   * Clear the cached active matrix tags
   */
  void clearActiveMatrixTags()
  {
    _active_matrix_tags.destroy();
    _matrix_tag_active = false;
  }

  /**
   * Get the MOOSE system
   * @returns The MOOSE system
   */
  ///@{
  SystemBase & getSystem() { return _system; }
  const SystemBase & getSystem() const { return _system; }

  ///@}
  /**
   * Get the libMesh DOF map
   * @returns The libMesh DOF map
   */
  const libMesh::DofMap & getDofMap() const;

  /**
   * Get the libMesh communicator
   * @returns The libMesh communicator
   */
  const Parallel::Communicator & getComm() const { return _comm; }

  /**
   * Get the list of local DOF indices to communicate
   * @returns The list of local DOF indices to communicate
   */
  const Array<Array<dof_id_type>> & getLocalCommList() const { return _local_comm_list; }

  /**
   * Get the list of ghost DOF indices to communicate
   * @returns The list of ghost DOF indices to communicate
   */
  const Array<Array<dof_id_type>> & getGhostCommList() const { return _ghost_comm_list; }

  /**
   * Get the sparisty pattern data
   * @returns The sparisty pattern data
   */
  const Sparsity & getSparsity() const { return _sparsity; }

  /**
   * Check whether a variable is active on a subdomain
   * @param var The variable number
   * @param subdomain The contiguous subdomain ID
   * @returns Whether the variable is active
   */
  KOKKOS_FUNCTION bool isVariableActive(unsigned int var, ContiguousSubdomainID subdomain) const
  {
    return _var_subdomain_active(var, subdomain);
  }

  /**
   * Check whether a residual tag is active
   * @param tag The residual tag
   * @returns Whether the residual tag is active
   */
  KOKKOS_FUNCTION bool isResidualTagActive(TagID tag) const { return _residual_tag_active[tag]; }

  /**
   * Check whether a matrix tag is active
   * @param tag The matrix tag
   * @returns Whether the matrix tag is active
   */
  KOKKOS_FUNCTION bool isMatrixTagActive(TagID tag) const { return _matrix_tag_active[tag]; }

  /**
   * Get the number of local DOFs
   * @returns The number of local DOFs
   */
  KOKKOS_FUNCTION dof_id_type getNumLocalDofs() const { return _num_local_dofs; }

  /**
   * Get the number of ghost DOFs
   * @returns The number of ghost DOFs
   */
  KOKKOS_FUNCTION dof_id_type getNumGhostDofs() const { return _num_ghost_dofs; }

  /**
   * Get the local DOF index of a variable for an element
   * @param elem The contiguous element ID
   * @param i The element-local DOF index
   * @param var The variable number
   * @returns The local DOF index
   */
  KOKKOS_FUNCTION dof_id_type getElemLocalDofIndex(ContiguousElementID elem,
                                                   unsigned int i,
                                                   unsigned int var) const
  {
    return _local_elem_dof_index[var](i, elem);
  }

  /**
   * Get the global DOF index of a variable for an element
   * @param elem The contiguous element ID
   * @param i The element-local DOF index
   * @param var The variable number
   * @returns The global DOF index
   */
  KOKKOS_FUNCTION dof_id_type getElemGlobalDofIndex(ContiguousElementID elem,
                                                    unsigned int i,
                                                    unsigned int var) const
  {
    return _local_to_global_dof_index[_local_elem_dof_index[var](i, elem)];
  }

  /**
   * Get the global DOF index of a local DOF index
   * @param dof The local DOF index
   * @returns The global DOF index
   */
  KOKKOS_FUNCTION dof_id_type localToGlobalDofIndex(dof_id_type dof) const
  {
    return _local_to_global_dof_index[dof];
  }

  /**
   * Get a tagged Kokkos vector
   * @param tag The vector tag
   * @returns The Kokkos vector
   */
  KOKKOS_FUNCTION Vector & getVector(TagID tag) const { return _vectors[tag]; }

  /**
   * Get a tagged Kokkos matrix
   * @param tag The matrix tag
   * @returns The Kokkos matrix
   */
  KOKKOS_FUNCTION Matrix & getMatrix(TagID tag) const { return _matrices[tag]; }

  /**
   * Get the DOF value of a tagged vector
   * @param dof The local DOF index
   * @param tag The vector tag
   * @returns The DOF value
   */
  KOKKOS_FUNCTION Real & getVectorDofValue(const dof_id_type dof, const TagID tag) const
  {
    return _vectors[tag][dof];
  }

  /**
   * Get an entry from a tagged matrix
   * @param row The local row index
   * @param col The global column index
   * @param tag The matrix tag
   * @returns The entry from the tagged matrix
   */
  KOKKOS_FUNCTION Real & getMatrixValue(dof_id_type row, dof_id_type col, TagID tag) const
  {
    return _matrices[tag](row, col);
  }

#endif

protected:
  /**
   * Reference of the MOOSE system
   */
  SystemBase & _system;

  /**
   * Reference of the MOOSE mesh
   */
  const MooseMesh & _mesh;

  /**
   * Reference of the libMesh DOF map
   */
  const libMesh::DofMap & _dof_map;

  /**
   * Reference of the libMesh communicator
   */
  const Parallel::Communicator & _comm;

  /**
   * Number of variables
   */
  const unsigned int _num_vars;

  /**
   * Number of local DOFs
   */
  const dof_id_type _num_local_dofs;

  /**
   * Number of ghost DOFs
   */
  const dof_id_type _num_ghost_dofs;

  /**
   * Kokkos vectors and matrices on device
   */
  ///@{
  Array<Vector> _vectors;
  Array<Matrix> _matrices;
  ///@}

  /**
   * Local element DOF indices of each variable
   */
  Array<Array2D<dof_id_type>> _local_elem_dof_index;

  /**
   * Map from local DOF index to global DOF index
   */
  Array<dof_id_type> _local_to_global_dof_index;

  /**
   * Maximum number of DOFs per element for each variable
   */
  Array<unsigned int> _max_dofs_per_elem;

  /**
   * Whether each variable is active on subdomains
   */
  Array2D<bool> _var_subdomain_active;

  /**
   * List of active variable numbers
   */
  Array<unsigned int> _active_variables;

  /**
   * List of active tags
   */
  ///@{
  Array<TagID> _active_solution_tags;
  Array<TagID> _active_residual_tags;
  Array<TagID> _active_matrix_tags;
  ///@}

  /**
   * Flag whether each tag is active
   */
  ///@{
  Array<bool> _residual_tag_active;
  Array<bool> _matrix_tag_active;
  ///@}

  /**
   * List of DOFs to send and receive
   */
  ///@{
  Array<Array<dof_id_type>> _local_comm_list;
  Array<Array<dof_id_type>> _ghost_comm_list;
  ///@}

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
   * Setup sparsity data
   */
  void setupSparsity();

  /**
   * Matrix sparsity pattern data
   */
  Sparsity _sparsity;
};

#ifdef MOOSE_KOKKOS_SCOPE
#define MakeSystemHolderMethods(SystemTypeName)                                                    \
  KOKKOS_FUNCTION const Array<SystemTypeName> & kokkosSystems() const                              \
  {                                                                                                \
    KOKKOS_IF_ON_HOST(return _systems_host;)                                                       \
    return _systems_device;                                                                        \
  }                                                                                                \
  Array<SystemTypeName> & kokkosSystems() { return _systems_host; }                                \
  KOKKOS_FUNCTION const SystemTypeName & kokkosSystem(unsigned int sys) const                      \
  {                                                                                                \
    KOKKOS_IF_ON_HOST(return _systems_host[sys];)                                                  \
    return _systems_device[sys];                                                                   \
  }                                                                                                \
  SystemTypeName & kokkosSystem(unsigned int sys) { return _systems_host[sys]; }
#else
#define MakeSystemHolderMethods(SystemTypeName)
#endif

/**
 * The Kokkos interface that holds the host reference of the Kokkos systems and copies it to device
 * during parallel dispatch.
 * Maintains synchronization between host and device Kokkos systems and provides access to the
 * appropriate Kokkos systems depending on the architecture.
 */
// clang-format off
#define MakeHolder(SystemTypeName)                                                                 \
  class SystemTypeName##Holder                                                                     \
  {                                                                                                \
  public:                                                                                          \
    SystemTypeName##Holder(Array<SystemTypeName> & systems)                                        \
      : _systems_host(systems), _systems_device(systems)                                           \
    {                                                                                              \
    }                                                                                              \
    SystemTypeName##Holder(const SystemTypeName##Holder & holder)                                  \
      : _systems_host(holder._systems_host), _systems_device(holder._systems_host)                 \
    {                                                                                              \
    }                                                                                              \
    MakeSystemHolderMethods(SystemTypeName)                                                        \
  private:                                                                                         \
    Array<SystemTypeName> & _systems_host;                                                         \
    const Array<SystemTypeName> _systems_device;                                                   \
  }
// clang-format on

MakeHolder(System);
} // namespace Moose::Kokkos
