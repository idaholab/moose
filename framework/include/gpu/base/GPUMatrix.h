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

#include "libmesh/petsc_matrix.h"

namespace Moose
{
namespace Kokkos
{

class System;

class Matrix
{
private:
  // The PETSc matrix
  Mat _matrix = PETSC_NULLPTR;
  // Number of rows
  PetscCount _nr = 0;
  // Column index vector
  Array<PetscInt> _col_idx;
  // Row index vector
  Array<PetscInt> _row_idx;
  // Row pointer vector
  Array<PetscInt> _row_ptr;
  // Nonzero value vector
  Array<PetscScalar> _val;
  // Flag whether the PETSc matrix is a host matrix
  bool _is_host = false;
  // Flag whether the matrix was allocated
  bool _is_alloc = false;

#ifdef MOOSE_KOKKOS_SCOPE
public:
  // Destructor
  ~Matrix() { destroy(); }

public:
  // Whether the matrix was allocated
  bool isAlloc() { return _is_alloc; }
  // Create from libMesh PetscMatrix
  void create(libMesh::SparseMatrix<PetscScalar> & matrix, const System & system);
  // Free all the matrix data
  void destroy();
  // Copy from device buffer to PETSc matrix
  void close();

public:
  // Zero one row
  KOKKOS_FUNCTION void zero(PetscInt i)
  {
    for (PetscInt j = _row_ptr[i]; j < _row_ptr[i + 1]; ++j)
      _val[j] = 0;
  }

private:
  KOKKOS_FUNCTION PetscInt find(PetscInt i,
                                PetscInt j,
                                const Array<PetscInt> & col_idx,
                                const Array<PetscInt> & row_ptr) const
  {
    auto left = col_idx.data() + row_ptr[i];
    auto right = col_idx.data() + row_ptr[i + 1] - 1;

    while (left <= right)
    {
      auto mid = left + (right - left) / 2;

      if (*mid == j)
        return mid - col_idx.data();
      else if (*mid < j)
        left = mid + 1;
      else
        right = mid - 1;
    }

    return -1;
  }

public:
  KOKKOS_FUNCTION PetscScalar & operator()(PetscInt i, PetscInt j) const
  {
    KOKKOS_ASSERT(i < _nr);

    auto idx = find(i, j, _col_idx, _row_ptr);

    KOKKOS_ASSERT(idx != -1)

    return _val[idx];
  }

public:
  // Scalar assignment operator
  auto & operator=(const PetscScalar & scalar)
  {
    _val = scalar;

    return *this;
  }
#endif
};

} // namespace Kokkos
} // namespace Moose
