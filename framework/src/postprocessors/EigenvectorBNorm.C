//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EigenvectorBNorm.h"

// MOOSE includes
#include "EigenProblem.h"
#include "NonlinearEigenSystem.h"

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_SLEPC
#include "libmesh/petsc_matrix.h"
#include <slepceps.h>
#endif

#include <cmath>

registerMooseObject("MooseApp", EigenvectorBNorm);

#ifdef LIBMESH_HAVE_SLEPC
namespace
{
/**
 * @return The quadratic form phi^T B phi, a collective operation
 * @param b_phi Work vector, allocated here when it does not match the layout of \p phi. It is
 * entirely overwritten by the multiplication, so it needs no zeroing between calls
 */
Real
quadraticForm(const libMesh::PetscMatrix<Number> & b_operator,
              const NumericVector<Number> & phi,
              std::unique_ptr<NumericVector<Number>> & b_phi)
{
  // The layout changes when the system is reinitialized, e.g. by adaptivity, and it differs between
  // the condensed and the full degree of freedom layout
  if (!b_phi || b_phi->size() != phi.size() || b_phi->local_size() != phi.local_size())
    b_phi = phi.zero_clone();

  b_operator.vector_mult(*b_phi, phi);
  return phi.dot(*b_phi);
}

/// Remedy shared by the errors about an eigenvalue solve that provides no assembled B matrix
const char * const assembled_b_remedy =
    "Use a solve type that assembles B, i.e. a linear eigen solve type such as KRYLOVSCHUR or "
    "JACOBI_DAVIDSON, or PJFNKMO.";
}
#endif

InputParameters
EigenvectorBNorm::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Computes the B-norm sqrt(phi^T B phi) of the current eigenvector of the nonlinear eigen "
      "system, where B is the operator the eigen solver solved with.");
  // The normalization of an eigenvector is performed on LINEAR, so this postprocessor has to be
  // executed there to be usable as the 'normalization' postprocessor of an eigenvalue executioner
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_END};
  return params;
}

EigenvectorBNorm::EigenvectorBNorm(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _nl_eigen(dynamic_cast<NonlinearEigenSystem *>(&_sys)),
    _b_norm_squared(0)
{
  if (!_nl_eigen)
    mooseError("Given system is not a NonlinearEigenSystem");
}

void
EigenvectorBNorm::execute()
{
  _b_norm_squared = 0;

#ifdef LIBMESH_HAVE_SLEPC
  // NEWTON evaluates the products with B as residuals and never assembles B, which only PJFNKMO
  // and the linear solve types do
  const auto & eigen_problem = cast_ref<const EigenProblem &>(_fe_problem);
  const auto & solver_params = eigen_problem.solverParams(_nl_eigen->number());
  if (solver_params._eigen_solve_type == Moose::EST_NEWTON)
    mooseError("EigenvectorBNorm requires an assembled B matrix, but the NEWTON eigenvalue solve "
               "never assembles B. ",
               assembled_b_remedy);

  const EPS eps = _nl_eigen->getEPS();

  // The operators are attached to the eigen solver by a solve only, and SLEPc errors out when they
  // are requested before that, so the norm is zero until the first solve has happened
  ST st;
  PetscInt n_operators;
  LibmeshPetscCall(EPSGetST(eps, &st));
  LibmeshPetscCall(STGetNumMatrices(st, &n_operators));
  if (!n_operators)
    return;

  // The nonlinear eigen solvers modify the operators during the solve, after which SLEPc refuses to
  // hand them out unless their state is reset, as the SLEPc callbacks of MOOSE do before they
  // retrieve the operators as well
  LibmeshPetscCall(STResetMatrixState(st));
  Mat A, B;
  LibmeshPetscCall(EPSGetOperators(eps, &A, &B));

  PetscBool b_is_shell = PETSC_FALSE;
  if (B)
    LibmeshPetscCall(PetscObjectTypeCompare((PetscObject)B, MATSHELL, &b_is_shell));
  if (b_is_shell)
    mooseError("EigenvectorBNorm requires an assembled B matrix, but this eigenvalue solve forms B "
               "matrix-free. ",
               assembled_b_remedy);

  // A condensed operator is posed on the non-condensed degrees of freedom only, so the eigenvector
  // has to be viewed through the same layout before it is multiplied by the operator
  auto & phi = _nl_eigen->solution();
  auto & eigen_sys = _nl_eigen->sys();
  std::unique_ptr<NumericVector<Number>> condensed_phi;
  if (_nl_eigen->dofMap().n_constrained_dofs())
    condensed_phi = phi.get_subvector(eigen_sys.local_non_condensed_dofs_vector);
  const NumericVector<Number> & eigen_phi = condensed_phi ? *condensed_phi : phi;

  // A standard eigenproblem has no B operator, i.e. B is the identity and the B-norm is the l2 norm
  if (!B)
  {
    const Real l2_norm = eigen_phi.l2_norm();
    _b_norm_squared = l2_norm * l2_norm;
  }
  else
  {
    // Between a mesh change and the next solve, the eigen solver still holds the operators of the
    // previous mesh, and a nonlinear solve may evaluate this before it first assembles B, which
    // must not be closed early as that discards its preallocation. The norm is zero then, as it is
    // before the first solve
    PetscBool b_assembled;
    PetscInt n_cols, n_local_cols;
    LibmeshPetscCall(MatAssembled(B, &b_assembled));
    LibmeshPetscCall(MatGetSize(B, nullptr, &n_cols));
    LibmeshPetscCall(MatGetLocalSize(B, nullptr, &n_local_cols));
    if (b_assembled && cast_int<numeric_index_type>(n_cols) == eigen_phi.size() &&
        cast_int<numeric_index_type>(n_local_cols) == eigen_phi.local_size())
    {
      const libMesh::PetscMatrix<Number> b_operator(B, comm());
      _b_norm_squared = quadraticForm(b_operator, eigen_phi, _b_phi);

      // PJFNKMO assembles B without the sign flip of 'negative_sign_eigen_kernel' and flips the
      // product B phi instead, whereas the other solve types flip B itself
      if (solver_params._eigen_matrix_vector_mult && eigen_problem.negativeSignEigenKernel())
        _b_norm_squared = -_b_norm_squared;
    }
  }

  if (condensed_phi)
    phi.restore_subvector(std::move(condensed_phi), eigen_sys.local_non_condensed_dofs_vector);
#endif
}

Real
EigenvectorBNorm::getValue() const
{
  if (_b_norm_squared < 0)
    mooseError("B is not positive definite for this eigenvector; check "
               "`negative_sign_eigen_kernel` and the sign of the eigen kernels");

  return std::sqrt(_b_norm_squared);
}
