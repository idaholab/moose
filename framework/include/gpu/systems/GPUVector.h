//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUArray.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/dof_map.h"

namespace Moose
{
namespace Kokkos
{

class System;

/**
 * The Kokkos wrapper class for PETSc vector
 */
class Vector
{
public:
  /**
   * Default constructor
   */
  Vector() = default;
  /**
   * Destructor
   */
  ~Vector() { destroy(); }
  /**
   * Free all data and reset
   */
  void destroy();

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get whether the vector was allocated
   * @returns Whether the vector was allocated
   */
  bool isAlloc() const { return _is_alloc; }
  /**
   * Create the vector from a libMesh PetscVector
   * @param vector The libMesh PetscVector
   * @param system The Kokkos system
   * @param assemble Whether the vector will be assembled
   */
  void create(libMesh::NumericVector<PetscScalar> & vector, const System & system, bool assemble);
  /**
   * Copy from/to the libMesh PetscVector
   */
  void copy(const MemcpyKind dir = MemcpyKind::HOST_TO_DEVICE);
  /**
   * Restore the underlying PETSc vector
   */
  void restore();
  /**
   * Assemble the underlying PETSc vector
   */
  void close();

  /**
   * Get an entry with a given index
   * @param i The entry index local to this process
   * @returns The reference of the entry
   */
  KOKKOS_FUNCTION PetscScalar & operator()(dof_id_type i) const
  {
    return i < _local.size() ? _local[i] : _ghost(i);
  }
  /**
   * Get an entry with a given index
   * @param i The entry index local to this process
   * @returns The reference of the entry
   */
  KOKKOS_FUNCTION PetscScalar & operator[](dof_id_type i) const
  {
    return i < _local.size() ? _local[i] : _ghost(i);
  }
  /**
   * Assign a scalar value uniformly
   * @param scalar The scalar value to be assigned
   */
  auto & operator=(PetscScalar scalar)
  {
    _local = scalar;
    _ghost = scalar;

    return *this;
  }

  /**
   * Kokkos functions for direct assembly on device
   */
  ///@{
  struct PackBuffer
  {
  };
  struct UnpackBuffer
  {
  };

  KOKKOS_FUNCTION void operator()(PackBuffer, const PetscCount tid) const;
  KOKKOS_FUNCTION void operator()(UnpackBuffer, const PetscCount tid) const;
  ///@}
#endif

private:
  /**
   * Data for direct assembly on device
   */
  ///@{
  struct DeviceAssembly
  {
    /**
     * List of DOFs to send/receive for each process
     */
    Array<Array<libMesh::dof_id_type>> list;
    /**
     * Number of DOFs to send/receive for each process
     */
    Array<int> count;
    /**
     * Starting offset of each process into the communication buffer
     */
    Array<int> offset;
    /**
     * Communication buffer
     */
    Array<PetscScalar> buffer;
    /**
     * Allocate data
     */
    void create(const Array<Array<libMesh::dof_id_type>> & list);
    /**
     * Free data
     */
    void destroy();
  };

  DeviceAssembly _send;
  DeviceAssembly _recv;

  unsigned int _current_proc;
  ///@}

  /**
   * PETSc vectors
   */
  ///@{
  Vec _global_vector = PETSC_NULLPTR;
  Vec _local_vector = PETSC_NULLPTR;
  ///@}
  /**
   * Raw data of local PETSc vector
   */
  PetscScalar * _array = PETSC_NULLPTR;
  /**
   * Pointer to the Kokkos system
   */
  const System * _system;
  /**
   * Pointer to the libMesh communicator
   */
  const libMesh::Parallel::Communicator * _comm = nullptr;
  /**
   * Data vectors on device
   */
  ///@{
  Array<PetscScalar> _local;
  Array<PetscScalar> _ghost;
  ///@}
  /**
   * Flag whether the vector will be assembled
   */
  bool _assemble = false;
  /**
   * Flag whether the PETSc vector is ghosted
   */
  bool _is_ghosted = false;
  /**
   * Flag whether the PETSc vector is a host vector
   */
  bool _is_host = false;
  /**
   * Flag whether the vector was allocated
   */
  bool _is_alloc = false;
};

} // namespace Kokkos
} // namespace Moose
