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
  BuildEquationSystemOperator();

  if (GetEquationSystem()->GetTestVarNames().size() > 1)
    mooseError("Eigenproblems are only supported in single-variable systems");

  auto eigensolver = std::dynamic_pointer_cast<MFEMEigensolverBase>(_problem_data.jacobian_solver);
  eigensolver->setMassMatrix(_mass_rhs);
  eigensolver->setOperator(GetEquationSystem()->_jacobian);
  eigensolver->solve();
  GetEquationSystem()->RecoverEigenproblemSolution(_problem_data.eigen_gridfunctions,
                                                   eigensolver.get());
}

void
EquationSystemEigenproblemOperator::BuildEquationSystemOperator()
{
  GetEquationSystem()->BuildEquationSystem();
  GetEquationSystem()->BuildEigenproblemJacobian(_true_x, _mass_rhs);
}

} // namespace Moose::MFEM

#endif
