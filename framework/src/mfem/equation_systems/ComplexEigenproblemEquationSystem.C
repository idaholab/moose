//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ComplexEigenproblemEquationSystem.h"
#include "MFEMEigensolverBase.h"
#include "MFEMEigenproblemBase.h"
#include "libmesh/int_range.h"

namespace Moose::MFEM
{

void
ComplexEigenproblemEquationSystem::ApplyEssentialBCs()
{
  _ess_tdof_lists.resize(1);
  mfem::ParComplexGridFunction & trial_gf = *(_cmplx_var_ess_constraints.at(0));
  _global_ess_markers.SetSize(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max());
  _global_ess_markers = 0;
  trial_gf.Update();
  static_cast<mfem::Vector &>(trial_gf) = _complex_gfuncs->GetRef(_trial_var_names.at(0));
  // Set constrained DoF values on user-declared essential boundaries and collect their markers
  ApplyComplexEssentialBC(_trial_var_names.at(0), trial_gf, _global_ess_markers);
  trial_gf.FESpace()->GetEssentialTrueDofs(_global_ess_markers, _ess_tdof_lists.at(0));
}

void
ComplexEigenproblemEquationSystem::FormEigenproblemMatrix(mfem::OperatorHandle & op)
{
  auto & test_var_name = _test_var_names.at(0);
  auto slf = _slfs.Get(test_var_name);

  // Set the real-block diagonal to 1 on essential DoFs. The imaginary-block diagonal must stay 0 so
  // each essential DoF's sub-block of the monolithic [[Re,-Im],[Im,Re]] system is the identity and
  // the assembled matrix remains symmetric for the real-valued solve.
  slf->real().EliminateEssentialBCDiag(_global_ess_markers, 1.0);
  slf->imag().EliminateEssentialBCDiag(_global_ess_markers, 0.0);
  slf->Finalize();
  std::unique_ptr<mfem::ComplexHypreParMatrix> cmat(slf->ParallelAssemble());
  mfem::HypreParMatrix * sysmat = cmat->GetSystemMatrix();

  // LOBPCG/AME require a symmetric operator. The monolithic system is symmetric only if the form is
  // Hermitian.
  std::unique_ptr<mfem::HypreParMatrix> sysmat_t(sysmat->Transpose());
  std::unique_ptr<mfem::HypreParMatrix> asymmetry(mfem::Add(1.0, *sysmat, -1.0, *sysmat_t));
  const mfem::real_t norm = sysmat->FNorm();
  const mfem::real_t asym = asymmetry->FNorm();
  if (norm > 0 && asym > 1e-10 * norm)
    mooseError("Assembled complex eigenproblem operator is not symmetric (relative asymmetry ",
               asym / norm,
               "). Current eigensolvers require a Hermitian form: a symmetric real part and an antisymmetric "
               "imaginary part. Check the selected kernels.");

  op.Reset(sysmat);
}

void
ComplexEigenproblemEquationSystem::FormMassMatrix(mfem::OperatorHandle & op)
{
  mfem::ParFiniteElementSpace * fespace = _test_pfespaces.at(0);
  std::unique_ptr<mfem::ParSesquilinearForm> m = std::make_unique<mfem::ParSesquilinearForm>(fespace);

  const bool use_matrix = _eigen_problem.rhsCoefficientIsMatrix();
  if (fespace->GetTypicalFE()->GetRangeType() == mfem::FiniteElement::SCALAR)
  {
    if (use_matrix)
      mooseError("A matrix rhs_coefficient cannot be used with a scalar finite element space.");
    m->AddDomainIntegrator(new mfem::MassIntegrator(_eigen_problem.getRHSCoefficient()), nullptr);
  }
  else
    m->AddDomainIntegrator(
        use_matrix ? new mfem::VectorFEMassIntegrator(_eigen_problem.getRHSMatrixCoefficient())
                   : new mfem::VectorFEMassIntegrator(_eigen_problem.getRHSCoefficient()),
        nullptr);

  m->Assemble();
  // Shift the eigenvalue corresponding to eliminated dofs to a large value. The BC DoFs on the
  // stiffness matrix are set to 1 and the mass matrix BC DoFs are set to a small value eps, such
  // that the eigenvalues associated with these DOFs are ~1/eps.
  // The imaginary part of the mass matrix is zero, is elimination needed?
  m->real().EliminateEssentialBCDiag(_global_ess_markers, std::numeric_limits<mfem::real_t>::min());
  m->Finalize();
  std::unique_ptr<mfem::ComplexHypreParMatrix> cmat(m->ParallelAssemble());
  op.Reset(cmat->GetSystemMatrix());
}

void
ComplexEigenproblemEquationSystem::BuildEigenproblemJacobian(mfem::BlockVector & trueX,
                                                      mfem::OperatorHandle & massRHS)
{
  mooseAssert(_test_var_names.size() == 1 && (_test_var_names.size() == _trial_var_names.size()) &&
                  (_test_var_names.at(0) == _trial_var_names.at(0)),
              "Eigensolve is only supported for single-variable, square systems");

  height = trueX.Size();
  width = trueX.Size();
  ApplyEssentialBCs();
  FormEigenproblemMatrix(_jacobian);
  FormMassMatrix(massRHS);
}

} // namespace Moose::MFEM

#endif
