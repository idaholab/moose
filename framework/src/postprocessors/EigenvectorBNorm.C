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
  const EPS eps = _nl_eigen->getEPS();

  // The operators are attached to the eigen solver by a solve only, and SLEPc errors out when they
  // are requested before that, so the norm is zero until the first solve has happened
  ST st;
  PetscInt n_operators;
  LibmeshPetscCall(EPSGetST(eps, &st));
  LibmeshPetscCall(STGetNumMatrices(st, &n_operators));
  if (!n_operators)
    return;

  Mat A, B;
  LibmeshPetscCall(EPSGetOperators(eps, &A, &B));

  auto & phi = _nl_eigen->solution();

  // A standard eigenproblem has no B operator, i.e. B is the identity and the B-norm is the l2 norm
  if (!B)
  {
    const Real l2_norm = phi.l2_norm();
    _b_norm_squared = l2_norm * l2_norm;
    return;
  }

  PetscBool b_is_shell;
  LibmeshPetscCall(PetscObjectTypeCompare((PetscObject)B, MATSHELL, &b_is_shell));
  if (b_is_shell)
    mooseError("EigenvectorBNorm requires an assembled B matrix, but this eigenvalue solve forms B "
               "matrix-free. Use a solve type that assembles B, i.e. a linear eigen solve type "
               "such as KRYLOVSCHUR or JACOBI_DAVIDSON, or NEWTON or PJFNKMO.");

  const libMesh::PetscMatrix<Number> b_operator(B, comm());

  // A condensed operator is posed on the non-condensed degrees of freedom only, so the eigenvector
  // has to be viewed through the same layout before it is multiplied by the operator
  if (_nl_eigen->eigenOperatorsCondensed())
  {
    auto & eigen_sys = _nl_eigen->sys();
    auto condensed_phi = phi.get_subvector(eigen_sys.local_non_condensed_dofs_vector);
    _b_norm_squared = quadraticForm(b_operator, *condensed_phi, _b_phi);
    phi.restore_subvector(std::move(condensed_phi), eigen_sys.local_non_condensed_dofs_vector);
  }
  else
    _b_norm_squared = quadraticForm(b_operator, phi, _b_phi);
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
