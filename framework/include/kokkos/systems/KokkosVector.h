//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/dof_map.h"

#include <unordered_set>

namespace Moose::Kokkos
{

class DofSpace;

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
   * @param dof_space The DOF layout of the system the vector belongs to
   * @param assemble Whether the vector will be assembled
   * @param read_only Whether the vector is only read, which takes the underlying array through
   * PETSc's read-only accessor and so accepts a vector the caller has locked against writes
   */
  void create(libMesh::NumericVector<PetscScalar> & vector,
              const DofSpace & dof_space,
              bool assemble,
              bool read_only = false);
  /**
   * Copy from the host libMesh PetscVector
   */
  void copyToDevice();
  /**
   * Copy to the host libMesh PetscVector
   */
  void copyToHost();
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
    KOKKOS_ASSERT(!_read_only);

    return i < _local.size() ? _local[i] : _ghost(i);
  }
  /**
   * Get an entry with a given index
   * @param i The entry index local to this process
   * @returns The reference of the entry
   */
  KOKKOS_FUNCTION PetscScalar & operator[](dof_id_type i) const
  {
    KOKKOS_ASSERT(!_read_only);

    return i < _local.size() ? _local[i] : _ghost(i);
  }
  /**
   * Get an entry with a given index for reading, which is valid whether or not the vector is
   * read-only
   * @param i The entry index local to this process
   * @returns The const reference of the entry
   */
  KOKKOS_FUNCTION const PetscScalar & read(dof_id_type i) const
  {
    // A read-only vector whose values PETSc already holds on the device aliases that array, while
    // one PETSc holds on the host was copied into the owned device storage
    if (_read_only && !_is_host)
      return _local_read[i];

    return i < _local.size() ? _local[i] : _ghost(i);
  }
  /**
   * Whether an index past the locally owned degrees of freedom resolves in this vector. An
   * assembled vector holds the ghost degrees of freedom in a separate offset array, and a ghosted
   * vector holds them alongside the local ones; a vector that is neither has storage for the
   * locally owned degrees of freedom alone.
   * @returns Whether a ghost index resolves
   */
  bool addressesGhostDofs() const { return _assemble || _is_ghosted; }
  /**
   * Assign a scalar value uniformly
   * @param scalar The scalar value to be assigned
   */
  auto & operator=(PetscScalar scalar)
  {
    mooseAssert(!_read_only, "Kokkos vector error: cannot assign to a read-only vector.");

    _local = scalar;
    _ghost = scalar;

    return *this;
  }
#endif

private:
  /**
   * PETSc vectors
   */
  ///@{
  Vec _global_vector = PETSC_NULLPTR;
  Vec _local_vector = PETSC_NULLPTR;
  ///@}
  /**
   * Raw data of local PETSc vector, held through the writable accessor
   */
  PetscScalar * _array = PETSC_NULLPTR;
  /**
   * Raw data of local PETSc vector, held through the read-only accessor when the vector is
   * read-only. Aliases PETSc's storage; it is never owned here.
   */
  const PetscScalar * _read_array = PETSC_NULLPTR;
  /**
   * Pointer to the DOF layout of the system the vector belongs to
   */
  const DofSpace * _dof_space = nullptr;
  /**
   * Data vectors on device
   */
  ///@{
  Array<PetscScalar> _local;
  Array<PetscScalar> _ghost;
  ///@}
  /**
   * Local data on device for a read-only vector PETSc already holds on the device, aliasing PETSc's
   * array. Const so that the alias needs no cast and cannot be written through.
   */
  Array<const PetscScalar> _local_read;
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
   * Flag whether the vector is only read, in which case its array is held through PETSc's
   * read-only accessor
   */
  bool _read_only = false;
  /**
   * Flag whether the vector was allocated
   */
  bool _is_alloc = false;
  /**
   * The PETSc vectors the COO preallocation has been set on, and the DOF layout it was set from.
   * VecSetPreallocationCOO() is a setup call whose cost is proportional to the vector's local size
   * rather than to the number of contributions, and PETSc keeps its result on the vector, so
   * close() sets it once per vector and reuses it afterwards. An operator application is handed
   * whichever of its caller's work vectors is free, cycling among several, so every vector seen is
   * remembered rather than only the last. Vectors are identified by PETSc object id, which is
   * unique over the run, so an entry can never be matched by a later vector.
   */
  ///@{
  std::unordered_set<PetscObjectId> _coo_vector_ids;
  const DofSpace * _coo_dof_space = nullptr;
  ///@}
};

} // namespace Moose::Kokkos
