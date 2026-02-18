//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMatrix.h"
#include "KokkosSystem.h"

namespace Moose::Kokkos
{

void
Matrix::create(libMesh::SparseMatrix<PetscScalar> & matrix, const System & system)
{
  auto petsc_matrix = dynamic_cast<libMesh::PetscMatrix<PetscScalar> *>(&matrix);

  mooseAssert(petsc_matrix, "Kokkos matrix error: provided matrix is not a PetscMatrix.");

  PetscMemType mtype;
  LibmeshPetscCallQ(MatGetCurrentMemType(petsc_matrix->mat(), &mtype));
  const bool is_host = PetscMemTypeHost(mtype);

#ifndef MOOSE_ENABLE_KOKKOS_GPU
  if (!is_host)
    mooseError("PETSc matrices must be on host when Kokkos device capabilities are disabled.");
#endif

  if (_is_alloc)
    return;

  auto & sparsity = system.getSparsity();

  _matrix = petsc_matrix->mat();
  _nr = sparsity.row_ptr.size() - 1;
  _col_idx = sparsity.col_idx;
  _row_idx = sparsity.row_idx;
  _row_ptr = sparsity.row_ptr;
  _is_host = is_host;

  if (!_is_host)
    _val.createDevice(_col_idx.size());
  else
    _val.create(_col_idx.size());

  std::vector<PetscInt> col_idx(&_col_idx.begin(), &_col_idx.end());
  std::vector<PetscInt> row_idx(&_row_idx.begin(), &_row_idx.end());

  LibmeshPetscCallQ(
      MatSetPreallocationCOO(_matrix, col_idx.size(), row_idx.data(), col_idx.data()));

  _is_alloc = true;
}

void
Matrix::destroy()
{
  _matrix = PETSC_NULLPTR;
  _nr = 0;

  _col_idx.destroy();
  _row_idx.destroy();
  _row_ptr.destroy();
  _val.destroy();

  _is_host = false;
  _is_alloc = false;
}

void
Matrix::close()
{
  if (_is_host)
    _val.copyToHost();

  LibmeshPetscCallQ(
      MatSetValuesCOO(_matrix, _is_host ? _val.hostData() : _val.deviceData(), ADD_VALUES));
}

} // namespace Moose::Kokkos
