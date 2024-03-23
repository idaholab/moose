//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseEnum.h"
#include "ReporterName.h"
#include "JacobianContainer.h"
#include "NonlinearSystemBase.h"
#include "NonlinearSystem.h"
#include "libmesh/id_types.h"
#include "libmesh/distributed_vector.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/sparse_matrix.h"
#include <utility>
#include <vector>

registerMooseObject("StochasticToolsApp", JacobianContainer);

InputParameters
JacobianContainer::validParams()
{
  InputParameters params = SnapshotContainerBase::validParams();
  params.addClassDescription("Class responsible for collecting distributed jacobian row "
                             "concatenated vectors into a container. We append "
                             "a new distributed jacobian row concatenated at every execution.");

  params.addRequiredParam<TagName>(
      "tag_name", "Name of the matrix tag to collect the snapshot, defaults to the system matrix.");
  params.addRequiredParam<ReporterValueName>(
      "jac_indices_reporter_name", "Name of the reporter that will hold the sparse indicies.");

  return params;
}

JacobianContainer::JacobianContainer(const InputParameters & parameters)
  : SnapshotContainerBase(parameters),
    _sparse_ind(declareValueByName<std::vector<std::pair<dof_id_type, dof_id_type>>>(
        getParam<ReporterValueName>("jac_indices_reporter_name"), REPORTER_MODE_REPLICATED)),
    _nl_sys(_fe_problem.getNonlinearSystem(_nonlinear_system_number)),
    _tag_id(_fe_problem.getMatrixTagID(getParam<TagName>("tag_name")))
{
  if (getParam<ExecFlagEnum>("execute_on").contains(EXEC_NONLINEAR))
    paramError("execute_on", "Matrix will always be empty during NONLINEAR.");
}

// Function to collect a snapshot of the current state of the Jacobian matrix
std::unique_ptr<NumericVector<Number>>
JacobianContainer::collectSnapshot()
{
  _sparse_ind.clear();

  // Obtain a reference to the Jacobian matrix from the nonlinear system
  auto & jac = _nl_sys.getMatrix(_tag_id);

  // Retrieve the dimensions of the Jacobian matrix
  auto num_cols = jac.n();

  // Close the Jacobian matrix to finalize its state for NONLINEAR systems
  jac.close();

  std::vector<Real> nnz_vals;

  // Iterate over the rows of the Jacobian matrix within the local domain
  for (dof_id_type row = jac.row_start(); row < jac.row_stop(); row++)
  {
    // Temporary vectors to hold the non-zero values and column indices for the current row
    std::vector<Real> values;
    std::vector<dof_id_type> cols;

    // Extract the non-zero values and their column indices for the current row
    jac.get_row(row, cols, values);

    // Adjust the column indices based on the row, for a flattened vector representation
    for (auto & col : cols)
    {
      _sparse_ind.push_back(std::make_pair(row, col));
      col = row * num_cols + col; // Update column index
    }
    nnz_vals.insert(nnz_vals.end(), values.begin(), values.end());
  }

  // Now each processors has the entire vector.
  _communicator.allgather(_sparse_ind);

  // Calculate the total size of the non-zero elements across all processors
  dof_id_type total_size = nnz_vals.size();
  _communicator.sum(total_size);

  // Create distributed vectors for storing the non-zero values and their indices
  DistributedVector<Real> sparse_vector(_communicator, total_size, nnz_vals.size());

  // Vector to hold the distributed indices for the non-zero values
  std::vector<dof_id_type> dist_ind(nnz_vals.size());
  // Initialize the distributed indices with consecutive values starting from the first local index
  std::iota(dist_ind.begin(), dist_ind.end(), sparse_vector.first_local_index());

  // Add the non-zero values to the distributed vectors
  sparse_vector.add_vector(nnz_vals, dist_ind);

  // Return a clone of the flattened Jacobian vector containing the non-zero values
  return sparse_vector.clone();
}
