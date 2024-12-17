//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "SegregatedSolverUtils.h"
#include "PetscVectorReader.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"

// Libmesh includes
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/system.h"

namespace NS
{
namespace FV
{
void
relaxMatrix(SparseMatrix<Number> & matrix,
            const Real relaxation_parameter,
            NumericVector<Number> & diff_diagonal)
{
  PetscMatrix<Number> * mat = dynamic_cast<PetscMatrix<Number> *>(&matrix);
  mooseAssert(mat, "This should be a PetscMatrix!");
  PetscVector<Number> * diff_diag = dynamic_cast<PetscVector<Number> *>(&diff_diagonal);
  mooseAssert(diff_diag, "This should be a PetscVector!");

  // Zero the diagonal difference vector
  *diff_diag = 0;

  // Get the diagonal of the matrix
  mat->get_diagonal(*diff_diag);

  // Create a copy of the diagonal for later use and cast it
  auto original_diagonal = diff_diag->clone();

  // We cache the inverse of the relaxation parameter because doing divisions might
  // be more expensive for every row
  const Real inverse_relaxation = 1 / relaxation_parameter;

  // Now we loop over the matrix row by row and sum the absolute values of the
  // offdiagonal values, If these sums are larger than the diagonal entry,
  // we switch the diagonal value with the sum. At the same time we increase the
  // diagonal by dividing it with the relaxation parameter. So the new diagonal will be:
  // D* = 1/lambda*max(|D|,sum(|Offdiag|))
  // For more information see
  //
  // Juretic, Franjo. Error analysis in finite volume CFD. Diss.
  // Imperial College London (University of London), 2005.
  //
  // The trickery comes with storing everything in the diff-diagonal vector
  // to avoid the allocation and manipulation of a third vector
  const unsigned int local_size = matrix.row_stop() - matrix.row_start();
  std::vector<dof_id_type> indices(local_size);
  std::iota(indices.begin(), indices.end(), matrix.row_start());
  std::vector<Real> new_diagonal(local_size, 0.0);

  {
    PetscVectorReader diff_diga_reader(*diff_diag);
    for (const auto row_i : make_range(local_size))
    {
      const unsigned int global_index = matrix.row_start() + row_i;
      std::vector<numeric_index_type> indices;
      std::vector<Real> values;
      mat->get_row(global_index, indices, values);
      const Real abs_sum = std::accumulate(
          values.cbegin(), values.cend(), 0.0, [](Real a, Real b) { return a + std::abs(b); });
      const Real abs_diagonal = std::abs(diff_diga_reader(global_index));
      new_diagonal[row_i] = inverse_relaxation * std::max(abs_sum - abs_diagonal, abs_diagonal);
    }
  }
  diff_diag->insert(new_diagonal, indices);

  // Time to modify the diagonal of the matrix. TODO: add this function to libmesh
  LibmeshPetscCallA(mat->comm().get(), MatDiagonalSet(mat->mat(), diff_diag->vec(), INSERT_VALUES));
  mat->close();
  diff_diag->close();

  // Finally, we can create (D*-D) vector which is used for the relaxation of the
  // right hand side later
  diff_diag->add(-1.0, *original_diagonal);
}

void
relaxRightHandSide(NumericVector<Number> & rhs,
                   const NumericVector<Number> & solution,
                   const NumericVector<Number> & diff_diagonal)
{

  // We need a working vector here to make sure we don't modify the
  // (D*-D) vector
  auto working_vector = diff_diagonal.clone();
  working_vector->pointwise_mult(solution, *working_vector);

  // The correction to the right hand side is just
  // (D*-D)*old_solution
  // For more information see
  //
  // Juretic, Franjo. Error analysis in finite volume CFD. Diss.
  // Imperial College London (University of London), 2005.
  rhs.add(*working_vector);
  rhs.close();
}

void
relaxSolutionUpdate(NumericVector<Number> & vec_new,
                    const NumericVector<Number> & vec_old,
                    const Real relaxation_factor)
{
  // The relaxation is just u = lambda * u* + (1-lambda) u_old
  vec_new.scale(relaxation_factor);
  vec_new.add(1 - relaxation_factor, vec_old);
  vec_new.close();
}

void
limitSolutionUpdate(NumericVector<Number> & solution, const Real min_limit, const Real max_limit)
{
  PetscVector<Number> & solution_vector = dynamic_cast<PetscVector<Number> &>(solution);
  auto value = solution_vector.get_array();

  for (auto i : make_range(solution_vector.local_size()))
    value[i] = std::min(std::max(min_limit, value[i]), max_limit);

  solution_vector.restore_array();
}

Real
computeNormalizationFactor(const NumericVector<Number> & solution,
                           const SparseMatrix<Number> & mat,
                           const NumericVector<Number> & rhs)
{
  // This function is based on the description provided here:
  // @article{greenshields2022notes,
  // title={Notes on computational fluid dynamics: General principles},
  // author={Greenshields, Christopher J and Weller, Henry G},
  // journal={(No Title)},
  // year={2022}
  // }
  // so basically we normalize the residual with the following number:
  // sum(|Ax-Ax_avg|+|b-Ax_avg|)
  // where A is the system matrix, b is the system right hand side while x and x_avg are
  // the solution and average solution vectors

  // We create a vector for Ax_avg and Ax
  auto A_times_average_solution = solution.zero_clone();
  auto A_times_solution = solution.zero_clone();

  // Beware, trickery here! To avoid allocating unused vectors, we
  // first compute Ax_avg using the storage used for Ax, then we
  // overwrite Ax with the right value
  *A_times_solution = solution.sum() / solution.size();
  mat.vector_mult(*A_times_average_solution, *A_times_solution);
  mat.vector_mult(*A_times_solution, solution);

  // We create Ax-Ax_avg
  A_times_solution->add(-1.0, *A_times_average_solution);
  // We create Ax_avg - b (ordering shouldn't matter we will take absolute value soon)
  A_times_average_solution->add(-1.0, rhs);
  A_times_solution->abs();
  A_times_average_solution->abs();

  // Create |Ax-Ax_avg|+|b-Ax_avg|
  A_times_average_solution->add(*A_times_solution);

  // Since use the l2 norm of the solution vectors in the linear solver, we will
  // make this consistent and use the l2 norm of the vector. We add a small number to
  // avoid normalizing with 0.
  // TODO: Would be nice to see if we can do l1 norms in the linear solve.
  return (A_times_average_solution->l2_norm() + libMesh::TOLERANCE * libMesh::TOLERANCE);
}

void
constrainSystem(SparseMatrix<Number> & mx,
                NumericVector<Number> & rhs,
                const Real desired_value,
                const dof_id_type dof_id)
{
  // Modify the given matrix and right hand side. We use the matrix diagonal
  // to enforce the constraint instead of 1, to make sure we don't mess up the matrix conditioning
  // too much.
  if (dof_id >= mx.row_start() && dof_id < mx.row_stop())
  {
    Real diag = mx(dof_id, dof_id);
    rhs.add(dof_id, desired_value * diag);
    mx.add(dof_id, dof_id, diag);
  }
}

dof_id_type
findPointDoFID(const MooseVariableFieldBase & variable, const MooseMesh & mesh, const Point & point)
{
  // Find the element containing the point
  auto point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, mesh);
  point_locator->enable_out_of_mesh_mode();

  unsigned int var_num = variable.sys().system().variable_number(variable.name());

  // We only check in the restricted blocks, if needed
  const bool block_restricted =
      variable.blockIDs().find(Moose::ANY_BLOCK_ID) == variable.blockIDs().end();
  const Elem * elem =
      block_restricted ? (*point_locator)(point, &variable.blockIDs()) : (*point_locator)(point);

  // We communicate the results and if there is conflict between processes,
  // the minimum cell ID is chosen
  const dof_id_type elem_id = elem ? elem->id() : DofObject::invalid_id;
  dof_id_type min_elem_id = elem_id;
  variable.sys().comm().min(min_elem_id);

  if (min_elem_id == DofObject::invalid_id)
    mooseError("Variable ",
               variable.name(),
               " is not defined at ",
               Moose::stringify(point),
               "! Try alleviating block restrictions or using another point!");

  return min_elem_id == elem_id ? elem->dof_number(variable.sys().number(), var_num, 0)
                                : DofObject::invalid_id;
}

bool
converged(const std::vector<std::pair<unsigned int, Real>> & its_and_residuals,
          const std::vector<Real> & abs_tolerances)
{
  mooseAssert(its_and_residuals.size() == abs_tolerances.size(),
              "The number of residuals should (now " + std::to_string(its_and_residuals.size()) +
                  ") be the same as the number of tolerances (" +
                  std::to_string(abs_tolerances.size()) + ")!");

  bool converged = true;
  for (const auto system_i : index_range(its_and_residuals))
  {
    converged = converged && (its_and_residuals[system_i].second < abs_tolerances[system_i]);
    if (!converged)
      return converged;
  }
  return converged;
}

} // End FV namespace
} // End Moose namespace
