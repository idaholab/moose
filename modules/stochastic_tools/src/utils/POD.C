
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "POD.h"

namespace StochasticTools
{

POD::POD(const ParallelSolutionStorage * const parallel_storage,
         const std::string & extra_slepc_options,
         const Parallel::Communicator & comm)
  : _parallel_storage(parallel_storage),
    _extra_slepc_options(extra_slepc_options),
    _communicator(comm)
{
}

void
POD::computePOD(const VariableName & vname,
                std::vector<DenseVector<Real>> & left_basis_functions,
                std::vector<DenseVector<Real>> & right_basis_functions,
                std::vector<Real> & singular_values,
                const dof_id_type num_modes,
                const Real energy) const
{
  // Define the petsc matrix which needs and SVD, we will populate it using the snapshots
  Mat mat;

  // We make sure every rank knows how many global and local samples we have and how long the
  // snapshots are. At this point we assume that the snapshots are the same size so we don't
  // need to map them to a reference domain.
  dof_id_type local_rows = 0;
  dof_id_type snapshot_size = 0;
  dof_id_type global_rows = 0;
  if (_parallel_storage->getStorage().size())
  {
    local_rows = _parallel_storage->getStorage(vname).size();
    global_rows = local_rows;
    if (_parallel_storage->getStorage(vname).size())
      snapshot_size = _parallel_storage->getStorage(vname).begin()->second[0].size();
  }

  _communicator.sum(global_rows);
  _communicator.max(snapshot_size);

  // The Lanczos method is preferred for symmetric semi positive definite matrices
  // but it only works with sparse matrices at the moment (dense matrix leaks memory).
  // So we create a sparse matrix with the distribution given in ParallelSolutionStorage.
  // TODO: Figure out a way to use a dense matrix without leaking memory, that would
  // avoid the overhead of using a sparse format for a dense matrix
  PetscErrorCode ierr = MatCreateAIJ(_communicator.get(),
                                     local_rows,
                                     snapshot_size,
                                     global_rows,
                                     snapshot_size,
                                     _communicator.rank() == 0 ? snapshot_size : 0,
                                     NULL,
                                     _communicator.rank() == 0 ? 0 : snapshot_size,
                                     NULL,
                                     &mat);
  LIBMESH_CHKERR(ierr);

  // Check where the local rows begin in the matrix, we use these to convert from local to
  // global indices
  dof_id_type local_beg = 0;
  dof_id_type local_end = 0;
  MatGetOwnershipRange(mat, numeric_petsc_cast(&local_beg), numeric_petsc_cast(&local_end));

  unsigned int counter = 0;
  if (local_rows)
    for (const auto & row : _parallel_storage->getStorage(vname))
    {
      std::vector<PetscInt> rows(snapshot_size, (counter++) + local_beg);

      // Fill the column indices with 0,1,...,snapshot_size-1
      std::vector<PetscInt> columns(snapshot_size);
      std::iota(std::begin(columns), std::end(columns), 0);

      // Set the rows in the "sparse" matrix
      MatSetValues(mat,
                   1,
                   rows.data(),
                   snapshot_size,
                   columns.data(),
                   row.second[0].get_values().data(),
                   INSERT_VALUES);
    }

  // Assemble the matrix
  MatAssemblyBegin(mat, MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd(mat, MAT_FINAL_ASSEMBLY);

  SVD svd;
  SVDCreate(_communicator.get(), &svd);
  // Now we set the operators for our SVD objects
  SVDSetOperators(svd, mat, NULL);

  // We want the Lanczos method, might give the choice to the user
  // at some point
  SVDSetType(svd, SVDTRLANCZOS);
  ierr = PetscOptionsInsertString(NULL, _extra_slepc_options.c_str());
  LIBMESH_CHKERR(ierr);

  // Set the subspace size for the Lanczos method, we take twice as many
  // basis vectors as the requested number of POD modes. This guarantees in most of the case the
  // convergence of the singular triplets.
  SVDSetFromOptions(svd);
  SVDSetDimensions(
      svd, num_modes, std::min(2 * num_modes, global_rows), std::min(2 * num_modes, global_rows));

  // Compute the singular value triplets
  ierr = SVDSolve(svd);
  LIBMESH_CHKERR(ierr);

  // Check how many singular triplets converged
  PetscInt nconv;
  ierr = SVDGetConverged(svd, &nconv);
  LIBMESH_CHKERR(ierr);

  // We start extracting the basis functions and the singular values. The left singular
  // vectors are supposed to be all on the root processor
  // PetscReal sigma;
  PetscVector<Real> u(_communicator);
  u.init(snapshot_size);

  PetscVector<Real> v(_communicator);
  v.init(global_rows, local_rows, false, PARALLEL);

  left_basis_functions.clear();
  right_basis_functions.clear();
  singular_values.clear();

  singular_values.resize(nconv);
  // Fetch the singular value triplet and immediately save the singular value
  for (PetscInt j = 0; j < nconv; ++j)
  {
    ierr = SVDGetSingularTriplet(svd, j, &singular_values[j], NULL, NULL);
    LIBMESH_CHKERR(ierr);
  }
  // Determine how many modes we need
  unsigned int num_requested_modes = determineNumberOfModes(singular_values, num_modes, energy);
  // Only save the basis functions which are needed. We serialize the modes
  // on every processor so all of them have access to every mode.
  left_basis_functions.resize(num_requested_modes);
  right_basis_functions.resize(num_requested_modes);
  for (PetscInt j = 0; j < cast_int<PetscInt>(num_requested_modes); ++j)
  {
    SVDGetSingularTriplet(svd, j, NULL, v.vec(), u.vec());
    u.localize(left_basis_functions[j].get_values());
    v.localize(right_basis_functions[j].get_values());
  }
  MatDestroy(&mat);
  SVDDestroy(&svd);
}

dof_id_type
POD::determineNumberOfModes(const std::vector<Real> & singular_values,
                            const dof_id_type num_modes_compute,
                            const Real energy) const
{
  dof_id_type num_modes = 0;
  // We either use the number of modes defined by the user or the maximum number of converged
  // modes. We don't want to use modes which are unconverged.
  std::size_t num_requested_modes =
      std::min((std::size_t)num_modes_compute, singular_values.size());
  // Grab a cumulative sum of singular value squared
  std::vector<Real> ev_sum(singular_values.begin(), singular_values.begin() + num_requested_modes);
  std::partial_sum(ev_sum.cbegin(),
                   ev_sum.cend(),
                   ev_sum.begin(),
                   [](Real sum, Real ev) { return sum + ev * ev; });

  // Find the first element that satisfies the threshold
  const Real threshold = energy;
  for (num_modes = 0; num_modes < ev_sum.size(); ++num_modes)
    if (ev_sum[num_modes] / ev_sum.back() > 1 - threshold)
      break;

  return num_modes + 1;
}
}
