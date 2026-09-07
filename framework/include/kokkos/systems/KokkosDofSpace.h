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
#include "KokkosMesh.h"

#include "libmesh/communicator.h"

class MooseMesh;

namespace libMesh
{
class DofMap;
class System;
}

namespace Moose::Kokkos
{

/**
 * The device-side DOF layout of a libMesh system: how many DOFs this process owns and ghosts, the
 * map from an element's local DOF indices to system-local and global DOF indices, and the DOF
 * indices exchanged with each other process.
 *
 * A Kokkos Vector is created over a layout, and device kernels gather from and scatter into a
 * vector through one, so the layout is held apart from the tagged vectors, matrices and residual
 * objects a Moose::Kokkos::System also carries. A p-multigrid level space owns a layout and none of
 * the rest.
 */
class DofSpace : public MeshHolder
{
public:
  /**
   * Constructor. The system's DOFs must already be distributed, which is to say the equation
   * systems must be initialized.
   * @param mesh The MOOSE mesh
   * @param system The libMesh system
   */
  DofSpace(const MooseMesh & mesh, libMesh::System & system);

  /**
   * Defaulted copy constructor
   */
  DofSpace(const DofSpace & src) = default;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the libMesh DOF map
   * @returns The libMesh DOF map
   */
  const libMesh::DofMap & getDofMap() const { return _dof_map; }

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
#endif

protected:
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
   * List of DOFs to send and receive
   */
  ///@{
  Array<Array<dof_id_type>> _local_comm_list;
  Array<Array<dof_id_type>> _ghost_comm_list;
  ///@}

private:
  /**
   * Setup DOF data
   * @param system The libMesh system
   */
  void setupDofs(libMesh::System & system);
};

} // namespace Moose::Kokkos
