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

class GPUSystem;

class GPUVector
{
private:
  struct MPIBuffer
  {
    // List of DOFs to send/receive for each process
    GPUArray<GPUArray<libMesh::dof_id_type>> list;
    // Number of DOFs to send/receive for each process
    GPUArray<int> count;
    // Offset of each process in the buffer
    GPUArray<int> offset;
    // Send/receive buffer
    GPUArray<PetscScalar> buffer;
    // Allocate data
    void create(const GPUArray<GPUArray<libMesh::dof_id_type>> & list);
    // Free data
    void destroy();
  };

private:
  // The global PETSc vector
  Vec _vector = PETSC_NULLPTR;
  // The local PETSc vector
  Vec _local_vector = PETSC_NULLPTR;
  // The raw array of PETSc vector
  PetscScalar * _array = PETSC_NULLPTR;
  // Pointer to the GPU system
  const GPUSystem * _system;
  // Pointer to the libMesh communicator
  const libMesh::Parallel::Communicator * _comm = nullptr;
  // Send buffer
  MPIBuffer _send;
  // Receive buffer
  MPIBuffer _recv;
  // Data that is used locally
  GPUArray<PetscScalar> _local;
  // Data that should be sent to ghost processes
  GPUArray<PetscScalar> _ghost;
  // Flag whether the vector will be assembled
  bool _assemble = false;
  // Flag whether the vector is ghosted
  bool _is_ghosted = false;
  // Flag whether the PETSc vector is a host vector
  bool _is_host = false;
  // Flag whether the vector was allocated
  bool _is_alloc = false;

#ifdef MOOSE_GPU_SCOPE
public:
  /**
   * Destructor
   */
  ~GPUVector() { destroy(); }

public:
  /**
   * Check whether the vector was allocated
   */
  bool isAlloc() { return _is_alloc; }
  /**
   * Create this vector from a libMesh PetscVector
   * @param vector libMesh NumericVector that can be downcast to a PetscVector
   * @param system GPU system
   * @param assemble Whether the vector will be assembled
   */
  void create(libMesh::NumericVector<PetscScalar> & vector,
              const GPUSystem & system,
              bool assemble = false);
  /**
   * Copy from/to the libMesh PetscVector
   */
  void copy(GPUMemcpyKind dir = GPUMemcpyKind::HOST_TO_DEVICE);
  /**
   * Free all the vector data
   */
  void destroy();
  /**
   * Restore the PETSc vector
   */
  void restore();
  /**
   * Assemble the vector
   */
  void close();

public:
  // GPU function tags
  struct PackBuffer
  {
  };
  struct UnpackBuffer
  {
  };

  /**
   * The GPU function of packing data into MPI send buffer
   * @param tid Thread index
   */
  KOKKOS_FUNCTION void operator()(PackBuffer, const PetscCount tid) const;
  /**
   * The GPU function of unpacking data from MPI receive buffer
   * @param tid Thread index
   */
  KOKKOS_FUNCTION void operator()(UnpackBuffer, const PetscCount tid) const;

public:
  /**
   * Data access operator
   * @param i Local index
   */
  KOKKOS_FUNCTION PetscScalar & operator()(PetscInt i) const
  {
    return i < _local.size() ? _local[i] : _ghost(i);
  }
  /**
   * Data access operator
   * @param i Local index
   */
  KOKKOS_FUNCTION PetscScalar & operator[](PetscInt i) const
  {
    return i < _local.size() ? _local[i] : _ghost(i);
  }
  /**
   * Scalar assignment operator
   * @param value Scalar value to assign
   */
  auto & operator=(PetscScalar scalar)
  {
    _local = scalar;
    _ghost = scalar;

    return *this;
  }

private:
  // Current process being packed/unpacked
  unsigned int _current_proc;
#endif
};
