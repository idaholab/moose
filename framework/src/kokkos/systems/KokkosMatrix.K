//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMatrix.h"
#include "KokkosFESystem.h"

namespace Moose::Kokkos
{

void
Matrix::create(libMesh::SparseMatrix<PetscScalar> & matrix,
               const System & system,
               std::vector<PetscInt> & coo_i,
               std::vector<PetscInt> & coo_j)
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
  _row_ptr = sparsity.row_ptr;
  _is_host = is_host;

  // MatSetPreallocationCOO() sorts its index arrays in place and biases the row indices while doing
  // so, so it cannot be handed the sparsity arrays themselves: the column indices are read on
  // device by find() for the life of the run, and every matrix tag preallocates from the same
  // sparsity.  Filling the scratch arrays here keeps that cost behind the early return above, which
  // matters because this call is reached on every host-to-device synchronization.
  system.buildCooIndices(coo_i, coo_j);

  LibmeshPetscCallQ(MatSetPreallocationCOO(_matrix, coo_i.size(), coo_i.data(), coo_j.data()));

  if (!_is_host)
    _val.createDevice(_col_idx.size());
  else
    _val.create(_col_idx.size());

  _is_alloc = true;
}

void
Matrix::destroy()
{
  _matrix = PETSC_NULLPTR;
  _nr = 0;

  _col_idx.destroy();
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

  ::Kokkos::fence();
}

} // namespace Moose::Kokkos
