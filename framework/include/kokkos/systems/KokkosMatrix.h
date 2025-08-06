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

#ifdef MOOSE_KOKKOS_SCOPE
#include "KokkosUtils.h"
#endif

#include "libmesh/petsc_matrix.h"

namespace Moose
{
namespace Kokkos
{

class System;

/**
 * The Kokkos wrapper class for PETSc matrix
 */
class Matrix
{
public:
  /**
   * Default constructor
   */
  Matrix() = default;
  /**
   * Destructor
   */
  ~Matrix() { destroy(); }
  /**
   * Free all data and reset
   */
  void destroy();

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get whether the matrix was allocated
   * @returns Whether the matrix was allocated
   */
  bool isAlloc() const { return _is_alloc; }
  /**
   * Create the matrix from a libMesh PetscMatrix
   * @param matrix The libMesh PetscMatrix
   * @param system The Kokkos system
   */
  void create(libMesh::SparseMatrix<PetscScalar> & matrix, const System & system);
  /**
   * Assemble the underlying PETSc matrix
   */
  void close();

  /**
   * Zero a row
   * @param i The row index local to this process to be zeroed
   */
  KOKKOS_FUNCTION void zero(PetscInt i)
  {
    for (PetscInt j = _row_ptr[i]; j < _row_ptr[i + 1]; ++j)
      _val[j] = 0;
  }
  /**
   * Get an entry with given row and column indices
   * @param i The row index local to this process
   * @param j The global column index
   * @returns The reference of the element
   */
  KOKKOS_FUNCTION PetscScalar & operator()(PetscInt i, PetscInt j) const
  {
    auto idx = find(i, j);

    KOKKOS_ASSERT(idx != -1)

    return _val[idx];
  }
  /**
   * Assign a scalar value uniformly
   * @param scalar The scalar value to be assigned
   */
  auto & operator=(PetscScalar scalar)
  {
    _val = scalar;

    return *this;
  }
#endif

private:
#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the index of given row and column indices
   * @param i The row index local to this process
   * @param j The global column index
   * @returns The index into the nonzero vector, and -1 if not found
   */
  KOKKOS_FUNCTION PetscInt find(PetscInt i, PetscInt j) const;
#endif

  /**
   * PETSc matrix
   */
  Mat _matrix = PETSC_NULLPTR;
  /**
   * Number of rows local to this process
   */
  PetscCount _nr = 0;
  /**
   * CSR vectors on device
   */
  ///@{
  Array<PetscInt> _col_idx;
  Array<PetscInt> _row_idx;
  Array<PetscInt> _row_ptr;
  Array<PetscScalar> _val;
  ///@}
  /**
   * Flag whether the PETSc matrix is a host matrix
   */
  bool _is_host = false;
  /**
   * Flag whether the matrix was allocated
   */
  bool _is_alloc = false;
};

#ifdef MOOSE_KOKKOS_SCOPE
KOKKOS_FUNCTION inline PetscInt
Matrix::find(PetscInt i, PetscInt j) const
{
  KOKKOS_ASSERT(i < _nr);

  auto begin = _col_idx.data() + _row_ptr[i];
  auto end = _col_idx.data() + _row_ptr[i + 1];
  auto target = Utils::find(j, begin, end);

  if (target == end)
    return -1;
  else
    return target - _col_idx.data();
}
#endif

} // namespace Kokkos
} // namespace Moose
