//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PODMapping.h"
#include <slepcsvd.h>
#include "libmesh/parallel_object.h"
#include "libmesh/petsc_vector.h"
#include "MooseTypes.h"
#include <petscdmda.h>

registerMooseObject("StochasticToolsApp", PODMapping);

InputParameters
PODMapping::validParams()
{
  InputParameters params = MappingBase::validParams();
  params.addParam<UserObjectName>(
      "solution_storage", "The name of the storage reporter where the snapshots are located.");
  return params;
}

PODMapping::PODMapping(const InputParameters & parameters) : MappingBase(parameters)
{
  // UserObjectName parallel_storage_name = getParam<UserObjectName>("solution_storage");
  // /// Reference to FEProblemBase instance
  // FEProblemBase & feproblem = *parameters.get<FEProblemBase *>("_fe_problem_base");

  // std::vector<UserObject *> reporters;
  // feproblem.theWarehouse()
  //     .query()
  //     .condition<AttribSystem>("UserObject")
  //     .condition<AttribName>(parallel_storage_name)
  //     .queryInto(reporters);

  // if (reporters.empty())
  //   paramError(
  //       "solution_storage", "Unable to find reporter with name '", parallel_storage_name, "'");
  // else if (reporters.size() > 1)
  //   paramError("solution_storage",
  //              "We found more than one reporter with the name '",
  //              parallel_storage_name,
  //              "'");

  // _parallel_storage = dynamic_cast<ParallelSolutionStorage *>(reporters[0]);

  // if (!_parallel_storage)
  //   paramError("solution_storage",
  //              "The parallel storage reporter is not of type '",
  //              parallel_storage_name,
  //              "'");
}

void
PODMapping::buildMapping()
{
  mooseAssert(comm().verify(typeAndName()), "Not at same branch");

  UserObjectName parallel_storage_name = getParam<UserObjectName>("solution_storage");
  /// Reference to FEProblemBase instance
  FEProblemBase & feproblem = *_pars.get<FEProblemBase *>("_fe_problem_base");

  std::vector<UserObject *> reporters;
  feproblem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(parallel_storage_name)
      .queryInto(reporters);

  if (reporters.empty())
    paramError(
        "solution_storage", "Unable to find reporter with name '", parallel_storage_name, "'");
  else if (reporters.size() > 1)
    paramError("solution_storage",
               "We found more than one reporter with the name '",
               parallel_storage_name,
               "'");

  ParallelSolutionStorage * parallel_storage =
      dynamic_cast<ParallelSolutionStorage *>(reporters[0]);

  if (!parallel_storage)
    paramError("solution_storage",
               "The parallel storage reporter is not of type '",
               parallel_storage_name,
               "'");


  Mat mat;

  unsigned int local_rows = 0;
  unsigned int snapshot_size = 0;
  unsigned int global_rows = 0;
  if (parallel_storage->getStorage().size())
  {
    local_rows = parallel_storage->getStorage()[0].size();
    global_rows = local_rows;
    snapshot_size = parallel_storage->getStorage()[0][0][0]->size();
  }

  comm().sum(global_rows);
  comm().max(snapshot_size);

  // PetscErrorCode ierr = MatCreateDense(_communicator.get(), local_rows, snapshot_size, global_rows, snapshot_size, NULL, &mat);
  // LIBMESH_CHKERR(ierr);
  // ierr = MatSetUp(mat);
  // LIBMESH_CHKERR(ierr);

  PetscErrorCode ierr = MatCreateAIJ(_communicator.get(), local_rows, snapshot_size, global_rows, snapshot_size,
                                     processor_id() == 0 ? snapshot_size : 0, NULL,
                                     processor_id() == 0 ? 0 : snapshot_size, NULL, &mat);

  // MatSetOption(mat,MAT_NEW_NONZERO_LOCATIONS,PETSC_FALSE);

  LIBMESH_CHKERR(ierr);

  dof_id_type local_beg;
  dof_id_type local_end;

  MatGetOwnershipRange(mat, numeric_petsc_cast(&local_beg), numeric_petsc_cast(&local_end));

  std::cerr << local_rows << " " << local_beg << " " << local_end << std::endl;
  std::cerr << snapshot_size << std::endl;
  for (const auto & col_i : make_range(local_rows))
  {
      std::vector<PetscInt> rows(snapshot_size, col_i+local_beg);
      std::vector<PetscInt> columns = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      MatSetValues(mat, 1, rows.data(), snapshot_size,columns.data(),
                   parallel_storage->getStorage()[0][col_i][0]->get_values().data(), INSERT_VALUES);
  }

  MatAssemblyBegin(mat, MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd(mat, MAT_FINAL_ASSEMBLY);

  ierr = MatView(mat, PETSC_VIEWER_STDOUT_SELF);
  LIBMESH_CHKERR(ierr);

  SVD svd;
  SVDCreate(_communicator.get(), &svd);
  SVDSetOperators(svd, mat, NULL);
  SVDSetProblemType(svd, SVD_STANDARD);
  SVDSetType(svd, SVDTRLANCZOS);
  ierr = PetscOptionsInsertString(NULL, "-svd_monitor_all");
  LIBMESH_CHKERR(ierr);
  SVDSetFromOptions(svd);
  SVDSetDimensions(svd, 2, 4, 4);

  ierr = SVDSolve(svd);
  LIBMESH_CHKERR(ierr);
  // PetscInt nconv;
  // // ierr = SVDGetConverged(svd, &nconv);
  // // LIBMESH_CHKERR(ierr);

  SVDDestroy(&svd);
  MatDestroy(&mat);
}

void
PODMapping::map(const DenseVector<Real> & full_order_vector,
                std::vector<Real> & reduced_order_vector) const
{
  _console << "Something smart" << std::endl;
}

void
PODMapping::map(const NumericVector<Number> & full_order_vector,
                std::vector<Real> & reduced_order_vector) const
{
  _console << "Something smart" << std::endl;
}

void
PODMapping::inverse_map(const std::vector<Real> & reduced_order_vector,
                        std::vector<Real> & full_order_vector) const
{
  _console << "Something smart" << std::endl;
}
