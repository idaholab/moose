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
#include "KokkosAssembly.h"

#include "SystemBase.h"

#include "libmesh/communicator.h"

class MooseMesh;
class SystemBase;

namespace Moose::Kokkos
{

class NodalBCBase;

/**
 * The Kokkos system class. Each nonlinear and auxiliary system in MOOSE has a corresponding Kokkos
 * system.
 */
class System : public MeshHolder, public AssemblyHolder
{
public:
  /**
   * Constructor
   * @param system The associated MOOSE system
   */
  System(SystemBase & system);

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
   * Allocate the quadrature point vectors for active variable and tags and cache
   * quadrature point values
   */
  void reinit();
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
  auto & getSystem() { return _system; }
  const auto & getSystem() const { return _system; }
  ///@}
  /**
   * Get the libMesh DOF map
   * @returns The libMesh DOF map
   */
  const auto & getDofMap() const { return _system.dofMap(); }
  /**
   * Get the libMesh communicator
   * @returns The libMesh communicator
   */
  const auto & getComm() const { return _comm; }
  /**
   * Get the list of local DOF indices to communicate
   * @returns The list of local DOF indices to communicate
   */
  const auto & getLocalCommList() const { return _local_comm_list; }
  /**
   * Get the list of ghost DOF indices to communicate
   * @returns The list of ghost DOF indices to communicate
   */
  const auto & getGhostCommList() const { return _ghost_comm_list; }
  /**
   * Get the sparisty pattern data
   * @returns The sparisty pattern data
   */
  const auto & getSparsity() const { return _sparsity; }
  /**
   * Get the list of off-diagonal coupled variable numbers of a variable
   * @param var The variable number
   * @returns The list of off-diagonal coupled variable numbers
   */
  KOKKOS_FUNCTION const auto & getCoupling(unsigned int var) const { return _coupling[var]; }
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
   * Check whether a local DOF index is covered by a nodal BC
   * @param dof The local DOF index
   * @returns Whether the local DOF index is covered by a nodal BC
   */
  KOKKOS_FUNCTION bool hasNodalBC(dof_id_type dof) const
  {
    return _nbc_dof.isAlloc() && _nbc_dof[dof];
  }
  /**
   * Check whether a local DOF index is covered by a nodal BC for an extra residual tag
   * @param dof The local DOF index
   * @param tag The extra residual tag
   * @returns Whether the local DOF index is covered by a nodal BC
   */
  KOKKOS_FUNCTION bool hasNodalBCResidualTag(dof_id_type dof, TagID tag) const
  {
    return _nbc_residual_tag_dof[tag].isAlloc() && _nbc_residual_tag_dof[tag][dof];
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
   * Get the maximum number of DOFs per element of a variable
   * This number is local to each process
   * @param var The variable number
   * @returns The maximum number of DOFs per element
   */
  KOKKOS_FUNCTION unsigned int getMaxDofsPerElem(unsigned int var) const
  {
    return _max_dofs_per_elem[var];
  }
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
    return _local_elem_dof_index[var](elem, i);
  }
  /**
   * Get the local DOF index of a variable for a node
   * @param node The contiguous node ID
   * @param idx The node-local DOF index
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
    return _local_to_global_dof_index[_local_elem_dof_index[var](elem, i)];
  }
  /**
   * Get the global DOF index of a variable for a node
   * @param node The contiguous node ID
   * @param var The variable number
   * @returns The global DOF index
   */
  KOKKOS_FUNCTION dof_id_type getNodeGlobalDofIndex(ContiguousNodeID node, unsigned int var) const
  {
    return _local_to_global_dof_index[_local_node_dof_index[var][node]];
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
   * Get a tagged Kokkos vector
   * @param tag The vector tag
   * @returns The Kokkos vector
   */
  KOKKOS_FUNCTION auto & getVector(TagID tag) const { return _vectors[tag]; }
  /**
   * Get a tagged Kokkos matrix
   * @param tag The matrix tag
   * @returns The Kokkos matrix
   */
  KOKKOS_FUNCTION auto & getMatrix(TagID tag) const { return _matrices[tag]; }
  /**
   * Get the DOF value of a tagged vector
   * @param dof The local DOF index
   * @param tag The vector tag
   * @returns The DOF value
   */
  KOKKOS_FUNCTION Real & getVectorDofValue(dof_id_type dof, TagID tag) const
  {
    return _vectors[tag][dof];
  }
  /**
   * Get the quadrature value of a variable from a tagged vector
   * @param info The element information object
   * @param qp The global quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The quadrature value
   */
  KOKKOS_FUNCTION Real &
  getVectorQpValue(ElementInfo info, dof_id_type qp, unsigned int var, TagID tag) const
  {
    return _qp_solutions[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the quadrature gradient of a variable from a tagged vector
   * @param info The element information object
   * @param qp The global quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The quadrature gradient
   */
  KOKKOS_FUNCTION Real3 &
  getVectorQpGrad(ElementInfo info, dof_id_type qp, unsigned int var, TagID tag) const
  {
    return _qp_solutions_grad[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the face quadrature value of a variable from a tagged vector
   * @param info The element information object
   * @param side The side index
   * @param qp The local quadrature point index
   * @param var The vriable number
   * @param tag The vector tag
   * @returns The face quadrature value
   */
  KOKKOS_FUNCTION Real getVectorQpValueFace(const ElementInfo info,
                                            const unsigned int side,
                                            const unsigned int qp,
                                            const unsigned int var,
                                            const TagID tag) const;
  /**
   * Get the face quadrature gradient of a variable from a tagged vector
   * @param info The element information object
   * @param side The side index
   * @param jacobian The inverse Jacobian matrix
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The face quadrature gradient
   */
  KOKKOS_FUNCTION Real3 getVectorQpGradFace(const ElementInfo info,
                                            const unsigned int side,
                                            const Real33 jacobian,
                                            const unsigned int qp,
                                            const unsigned int var,
                                            const TagID tag) const;
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

  /**
   * Kokkos function for caching variable values on element quadrature points
   */
  KOKKOS_FUNCTION void operator()(const ThreadID tid) const;
#endif

  /**
   * CSR format sparsity data
   */
  struct Sparsity
  {
    Array<PetscInt> col_idx;
    Array<PetscInt> row_idx;
    Array<PetscInt> row_ptr;
  };

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
   * Check if the DOFs are covered by nodal BCs
   */
  void checkNodalBCs();
  /**
   * Get the list of DOFs covered by a nodal BC
   * @param nbc The Kokkos nodal BC object
   * @param dofs The flag whether each DOF is covered by the nodal BC
   */
  void getNodalBCDofs(const NodalBCBase * nbc, Array<bool> & dofs);

  /**
   * Kokkos thread object
   */
  Thread _thread;
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
   * Cached elemental quadrature values and gradients
   */
  ///@{
  Array<Array2D<Array<Real>>> _qp_solutions;
  Array<Array2D<Array<Real3>>> _qp_solutions_grad;
  ///@}
  /**
   * Local DOF index of each variable
   */
  ///@{
  Array<Array2D<dof_id_type>> _local_elem_dof_index;
  Array<Array<dof_id_type>> _local_node_dof_index;
  ///@}
  /**
   * Map from local DOF index to global DOF index
   */
  Array<dof_id_type> _local_to_global_dof_index;
  /**
   * Maximum number of DOFs per element for each variable
   */
  Array<unsigned int> _max_dofs_per_elem;
  /**
   * FE type ID of each variable
   */
  Array<unsigned int> _var_fe_types;
  /**
   * Whether each variable is active on subdomains
   */
  Array2D<bool> _var_subdomain_active;
  /**
   * Off-diagonal coupled variable numbers of each variable
   */
  Array<Array<unsigned int>> _coupling;

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
   * Flag whether each DOF is covered by a nodal BC
   */
  Array<bool> _nbc_dof;
  /**
   * Flag whether each DOF is covered by a nodal BC for extra residual tags
   */
  ///@{
  Array<Array<bool>> _nbc_residual_tag_dof;
  Array<Array<bool>> _nbc_matrix_tag_dof;
  ///@}
  /**
   * List of DOFs to send and receive
   */
  ///@{
  Array<Array<dof_id_type>> _local_comm_list;
  Array<Array<dof_id_type>> _ghost_comm_list;
  ///@}

  /**
   * Matrix sparsity pattern data
   */
  Sparsity _sparsity;
};

#ifdef MOOSE_KOKKOS_SCOPE
KOKKOS_FUNCTION inline Real
System::getVectorQpValueFace(
    ElementInfo info, unsigned int side, unsigned int qp, unsigned int var, TagID tag) const
{
  auto fe = _var_fe_types[var];
  auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
  auto & phi = kokkosAssembly().getPhiFace(info.subdomain, info.type, fe)(side);

  Real value = 0;

  for (unsigned int i = 0; i < n_dofs; ++i)
    value += getVectorDofValue(getElemLocalDofIndex(info.id, i, var), tag) * phi(i, qp);

  return value;
}
KOKKOS_FUNCTION inline Real3
System::getVectorQpGradFace(ElementInfo info,
                            unsigned int side,
                            Real33 jacobian,
                            unsigned int qp,
                            unsigned int var,
                            TagID tag) const
{
  auto fe = _var_fe_types[var];
  auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
  auto & grad_phi = kokkosAssembly().getGradPhiFace(info.subdomain, info.type, fe)(side);

  Real3 grad = 0;

  for (unsigned int i = 0; i < n_dofs; ++i)
    grad += getVectorDofValue(getElemLocalDofIndex(info.id, i, var), tag) *
            (jacobian * grad_phi(i, qp));

  return grad;
}
#endif

/**
 * The Kokkos interface that holds the host reference of the Kokkos systems and copies it to device
 * during parallel dispatch.
 * Maintains synchronization between host and device Kokkos systems and provides access to the
 * appropriate Kokkos systems depending on the architecture.
 */
class SystemHolder
{
public:
  /**
   * Constructor
   * @param systems The Kokkos systems
   */
  SystemHolder(Array<System> & systems) : _systems_host(systems), _systems_device(systems) {}
  /**
   * Copy constructor
   */
  SystemHolder(const SystemHolder & holder)
    : _systems_host(holder._systems_host), _systems_device(holder._systems_host)
  {
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the const reference of the Kokkos systems
   * @returns The const reference of the Kokkos systems depending on the architecture this function
   * is being called on
   */
  KOKKOS_FUNCTION const Array<System> & kokkosSystems() const
  {
    KOKKOS_IF_ON_HOST(return _systems_host;)

    return _systems_device;
  }
  /**
   * Get the writeable host reference of the Kokkos systems
   * @returns The writeable host reference of the Kokkos systems
   */
  Array<System> & kokkosSystems() { return _systems_host; }
  /**
   * Get the const reference of a Kokkos system
   * @param sys The system number
   * @returns The const reference of the Kokkos system depending on the architecture this function
   * is being called on
   */
  KOKKOS_FUNCTION const System & kokkosSystem(unsigned int sys) const
  {
    KOKKOS_IF_ON_HOST(return _systems_host[sys];)

    return _systems_device[sys];
  }
  /**
   * Get the writeable reference of a Kokkos system
   * @param sys The system number
   * @returns The writeable host reference of the Kokkos system
   */
  System & kokkosSystem(unsigned int sys) { return _systems_host[sys]; }
#endif

private:
  /**
   * Host reference of the Kokkos systems
   */
  Array<System> & _systems_host;
  /**
   * Device copy of the Kokkos systems
   */
  const Array<System> _systems_device;
};

} // namespace Moose::Kokkos
