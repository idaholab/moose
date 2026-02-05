//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "EquationSystemEigenproblemOperator.h"

namespace Moose::MFEM
{

void
EquationSystemEigenproblemOperator::Solve()
{
  GetEquationSystem()->BuildEigenproblemJacobian(_true_x, _mass_rhs);

  if (GetEquationSystem()->GetTestVarNames().size() > 1)
    mooseError("Eigenproblems are only supported in single-variable systems");

  _problem_data.jacobian_solver->updateSolver(
      *GetEquationSystem()->_blfs.Get(GetEquationSystem()->GetTestVarNames().at(0)),
      GetEquationSystem()->_ess_tdof_lists.at(0));

  auto eigensolver = std::dynamic_pointer_cast<MFEMEigensolverBase>(_problem_data.jacobian_solver);
  eigensolver->setOperator(*GetEquationSystem());
  eigensolver->setMassMatrix(*_mass_rhs);
  eigensolver->Solve();
  GetEquationSystem()->RecoverEigenproblemSolution(_problem_data.gridfunctions, eigensolver.get());

}

} // namespace Moose::MFEM

#endif
