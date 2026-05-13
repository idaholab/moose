//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "EigenproblemESProblemOperator.h"
#include "MFEMEigensolverBase.h"

namespace Moose::MFEM
{

void
EigenproblemESProblemOperator::Solve()
{
  BuildEquationSystemOperator();

  if (GetEquationSystem()->GetTestVarNames().size() > 1)
    mooseError("Eigenproblems are only supported in single-variable systems");

  auto eigensolver = std::dynamic_pointer_cast<MFEMEigensolverBase>(_problem_data.jacobian_solver);
  eigensolver->setMassMatrix(_mass_rhs);
  eigensolver->setOperator(GetEquationSystem()->_jacobian);
  eigensolver->solve();
  RecoverEigenproblemSolution(_problem_data.gridfunctions, eigensolver.get());
}

void
EigenproblemESProblemOperator::BuildEquationSystemOperator()
{
  GetEquationSystem()->BuildEquationSystem();
  GetEquationSystem()->BuildEigenproblemJacobian(_true_x, _mass_rhs);
}

void
EigenproblemESProblemOperator::RecoverEigenproblemSolution(
    Moose::MFEM::GridFunctions & gridfunctions, MFEMEigensolverBase * eigensolver)
{
  mfem::Array<mfem::real_t> eigenvalues;
  eigensolver->getEigenvalues(eigenvalues);

  const auto & trial_var_name = _trial_var_names.at(0);
  const auto & sep = _problem_data.mode_separator;

  // Distribute the zeroth mode onto the base variable
  gridfunctions.Get(trial_var_name)->Distribute(eigensolver->getEigenvector(0));

  for (int i = 0; i < eigenvalues.Size(); ++i)
    gridfunctions.Get(trial_var_name + sep + std::to_string(i))
        ->Distribute(eigensolver->getEigenvector(i));
}

} // namespace Moose::MFEM

#endif
