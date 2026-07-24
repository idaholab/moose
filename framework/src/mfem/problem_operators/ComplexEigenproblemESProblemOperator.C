//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ComplexEigenproblemESProblemOperator.h"
#include "MFEMEigensolverBase.h"

namespace Moose::MFEM
{

void
ComplexEigenproblemESProblemOperator::Solve()
{
  BuildEquationSystemOperator();

  if (GetEquationSystem()->GetTestVarNames().size() > 1)
    mooseError("Eigenproblems are only supported in single-variable systems");

  auto eigensolver =
      std::dynamic_pointer_cast<Moose::MFEM::EigensolverBase>(_problem_data.jacobian_solver);
  eigensolver->SetMassMatrix(_mass_rhs);
  eigensolver->SetOperator(GetComplexEigenproblemEquationSystem()->_jacobian);
  eigensolver->Solve();
  RecoverEigenproblemSolution(eigensolver.get());
}

void
ComplexEigenproblemESProblemOperator::BuildEquationSystemOperator()
{
  GetComplexEigenproblemEquationSystem()->BuildEquationSystem();
  GetComplexEigenproblemEquationSystem()->BuildEigenproblemJacobian(_true_x, _mass_rhs);
}

void
ComplexEigenproblemESProblemOperator::RecoverEigenproblemSolution(EigensolverBase * eigensolver)
{
  mfem::Array<mfem::real_t> eigenvalues;
  eigensolver->GetEigenvalues(eigenvalues);

  const auto & trial_var_name = _trial_var_names.at(0);
  const auto & sep = _problem_data.mode_separator;

  // The eigensolver works on the monolithic [real; imag] system, so each eigenvector is a length-2N
  // true vector that ParComplexGridFunction::Distribute splits back into real and imaginary parts.

  // Distribute the zeroth mode onto the base variable.
  _problem_data.cmplx_gridfunctions.Get(trial_var_name)->Distribute(eigensolver->GetEigenvector(0));

  for (int i = 0; i < eigenvalues.Size(); ++i)
    _problem_data.cmplx_gridfunctions.Get(trial_var_name + sep + std::to_string(i))
        ->Distribute(eigensolver->GetEigenvector(i));
}

} // namespace Moose::MFEM

#endif
