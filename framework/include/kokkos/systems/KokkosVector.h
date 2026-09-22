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

#include <petscsf.h>

#include <memory>

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

    return i < _local.size() ? _local[i] : _stash(i);
  }
  /**
   * Get an entry with a given index
   * @param i The entry index local to this process
   * @returns The reference of the entry
   */
  KOKKOS_FUNCTION PetscScalar & operator[](dof_id_type i) const
  {
    KOKKOS_ASSERT(!_read_only);

    return i < _local.size() ? _local[i] : _stash(i);
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

    return i < _local.size() ? _local[i] : _stash(i);
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
    _stash = scalar;

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
   * Data vectors on device.
   *
   * _local holds the entries this process owns, aliasing PETSc's array. For a vector read through a
   * ghosted layout it is sized to take the ghost entries after them as well, so reading an entry
   * another process owns is a read of _local.
   *
   * _stash holds contributions to rows another process owns, which close() reduces into their owners.
   * It exists only for a vector being assembled, and it is a write-side buffer rather than the
   * ghosting of a vector for reading: PETSc's own name for such a buffer is a stash.
   */
  ///@{
  Array<PetscScalar> _local;
  Array<PetscScalar> _stash;
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
   * The star forest close() reduces the off-process contributions through, and the DOF layout it was
   * built from. The pattern depends on the layout alone rather than on the vector, so one star forest
   * serves every vector this wrapper is created around and is rebuilt only when the layout changes.
   *
   * Ownership is shared rather than held directly because this class is a value type: it is copied on
   * the host and copied bytewise into device memory, so a member whose destructor frees a resource
   * would have every copy free what the original still refers to.
   */
  ///@{
  std::shared_ptr<PetscSF> _reduction;
  const DofSpace * _reduction_dof_space = nullptr;
  ///@}
};

} // namespace Moose::Kokkos
