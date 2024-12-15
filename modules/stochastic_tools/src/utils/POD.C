
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "POD.h"

using namespace libMesh;

namespace StochasticTools
{

#if PETSC_VERSION_LESS_THAN(3, 14, 0)

POD::POD(const ParallelSolutionStorage * const, const std::string &, const Parallel::Communicator &)
{
  mooseError("PETSc-3.14.0 or higher is required for using StochasticTools::POD.");
}

#else

POD::POD(const ParallelSolutionStorage * const parallel_storage,
         const std::string & extra_slepc_options,
         const Parallel::Communicator & comm)
  : _parallel_storage(parallel_storage),
    _extra_slepc_options(extra_slepc_options),
    _communicator(comm)
{
}

#endif

void
POD::computePOD(const VariableName & vname,
                std::vector<DenseVector<Real>> & left_basis_functions,
                std::vector<DenseVector<Real>> & right_basis_functions,
                std::vector<Real> & singular_values,
                const dof_id_type num_modes,
                const Real energy) const
{

#if !PETSC_VERSION_LESS_THAN(3, 14, 0)

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
    for (const auto & row : _parallel_storage->getStorage(vname))
    {
      local_rows += row.second.size();
      if (row.second.size())
        snapshot_size = row.second[0].size();
    }
    global_rows = local_rows;
  }

  _communicator.sum(global_rows);
  _communicator.max(snapshot_size);

  // Generally snapshot matrices are dense.
  LibmeshPetscCallA(
      _communicator.get(),
      MatCreateDense(
          _communicator.get(), local_rows, PETSC_DECIDE, global_rows, snapshot_size, NULL, &mat));

  // Check where the local rows begin in the matrix, we use these to convert from local to
  // global indices
  dof_id_type local_beg = 0;
  dof_id_type local_end = 0;
  LibmeshPetscCallA(
      _communicator.get(),
      MatGetOwnershipRange(mat, numeric_petsc_cast(&local_beg), numeric_petsc_cast(&local_end)));

  unsigned int counter = 0;
  if (local_rows)
    for (const auto & row : _parallel_storage->getStorage(vname))
    {
      // Adds each snap individually. For problems with multiple snaps per run.
      for (const auto & snap : row.second)
      {
        std::vector<PetscInt> rows(snapshot_size, (counter++) + local_beg);

        // Fill the column indices with 0,1,...,snapshot_size-1
        std::vector<PetscInt> columns(snapshot_size);
        std::iota(std::begin(columns), std::end(columns), 0);

        // Set the rows in the "sparse" matrix
        LibmeshPetscCallA(_communicator.get(),
                          MatSetValues(mat,
                                       1,
                                       rows.data(),
                                       snapshot_size,
                                       columns.data(),
                                       snap.get_values().data(),
                                       INSERT_VALUES));
      }
    }

  // Assemble the matrix
  LibmeshPetscCallA(_communicator.get(), MatAssemblyBegin(mat, MAT_FINAL_ASSEMBLY));
  LibmeshPetscCallA(_communicator.get(), MatAssemblyEnd(mat, MAT_FINAL_ASSEMBLY));

  SVD svd;
  LibmeshPetscCallA(_communicator.get(), SVDCreate(_communicator.get(), &svd));
  // Now we set the operators for our SVD objects
  LibmeshPetscCallA(_communicator.get(), SVDSetOperators(svd, mat, NULL));

  // Set the parallel operation mode to "DISTRIBUTED", default is "REDUNDANT"
  DS ds;
  LibmeshPetscCallA(_communicator.get(), SVDGetDS(svd, &ds));
  LibmeshPetscCallA(_communicator.get(), DSSetParallel(ds, DS_PARALLEL_DISTRIBUTED));

  // We want the Lanczos method, might give the choice to the user
  // at some point
  LibmeshPetscCallA(_communicator.get(), SVDSetType(svd, SVDTRLANCZOS));

  // Default is the transpose is explicitly created. This method is less efficient
  // computationally but better for storage
  LibmeshPetscCallA(_communicator.get(), SVDSetImplicitTranspose(svd, PETSC_TRUE));

  LibmeshPetscCallA(_communicator.get(),
                    PetscOptionsInsertString(NULL, _extra_slepc_options.c_str()));

  // Set the subspace size for the Lanczos method, we take twice as many
  // basis vectors as the requested number of POD modes. This guarantees in most of the case the
  // convergence of the singular triplets.
  LibmeshPetscCallA(_communicator.get(),
                    SVDSetDimensions(svd,
                                     num_modes,
                                     std::min(2 * num_modes, global_rows),
                                     std::min(2 * num_modes, global_rows)));

  // Gives the user the ability to override any option set before the solve.
  LibmeshPetscCallA(_communicator.get(), SVDSetFromOptions(svd));

  // Compute the singular value triplets
  LibmeshPetscCallA(_communicator.get(), SVDSolve(svd));

  // Check how many singular triplets converged
  PetscInt nconv;
  LibmeshPetscCallA(_communicator.get(), SVDGetConverged(svd, &nconv));

  // We start extracting the basis functions and the singular values.

  // Find the local size needed for u
  dof_id_type local_snapsize = 0;
  LibmeshPetscCallA(_communicator.get(),
                    MatGetLocalSize(mat, NULL, numeric_petsc_cast(&local_snapsize)));

  PetscVector<Real> u(_communicator);
  u.init(snapshot_size, local_snapsize, false, PARALLEL);

  PetscVector<Real> v(_communicator);
  v.init(global_rows, local_rows, false, PARALLEL);

  left_basis_functions.clear();
  right_basis_functions.clear();
  singular_values.clear();

  singular_values.resize(nconv);
  // Fetch the singular value triplet and immediately save the singular value
  for (PetscInt j = 0; j < nconv; ++j)
    LibmeshPetscCallA(_communicator.get(),
                      SVDGetSingularTriplet(svd, j, &singular_values[j], NULL, NULL));

  // Determine how many modes we need
  unsigned int num_requested_modes = determineNumberOfModes(singular_values, num_modes, energy);
  // Only save the basis functions which are needed. We serialize the modes
  // on every processor so all of them have access to every mode.
  left_basis_functions.resize(num_requested_modes);
  right_basis_functions.resize(num_requested_modes);
  for (PetscInt j = 0; j < cast_int<PetscInt>(num_requested_modes); ++j)
  {
    LibmeshPetscCallA(_communicator.get(), SVDGetSingularTriplet(svd, j, NULL, v.vec(), u.vec()));
    u.localize(left_basis_functions[j].get_values());
    v.localize(right_basis_functions[j].get_values());
  }
  LibmeshPetscCallA(_communicator.get(), MatDestroy(&mat));
  LibmeshPetscCallA(_communicator.get(), SVDDestroy(&svd));
#else
  // These variables would otherwise be unused
  libmesh_ignore(vname);
  libmesh_ignore(left_basis_functions);
  libmesh_ignore(right_basis_functions);
  libmesh_ignore(singular_values);
  libmesh_ignore(num_modes);
  libmesh_ignore(energy);
#endif
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
