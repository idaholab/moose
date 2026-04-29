//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMOperatorChebyshevSmoother.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", OperatorChebyshevSmoother);

namespace Moose::MFEM
{

InputParameters
OperatorChebyshevSmoother::validParams()
{
  InputParameters params = LinearSolverBase::validParams();
  params.addClassDescription(
      "Chebyshev polynomial smoother backed by mfem::OperatorChebyshevSmoother. "
      "Symmetric positive definite by construction; the maximum eigenvalue of D^{-1}A "
      "is estimated automatically via a power method. Suitable as a multigrid smoother "
      "without requiring a manual damping parameter.");
  params.addParam<int>("order", 2, "Degree of the Chebyshev polynomial (1–4).");
  return params;
}

OperatorChebyshevSmoother::OperatorChebyshevSmoother(const InputParameters & parameters)
  : LinearSolverBase(parameters), _order(getParam<int>("order"))
{
  constructSolver();
}

void
OperatorChebyshevSmoother::constructSolver()
{
  // Deferred: mfem::OperatorChebyshevSmoother requires the operator and its diagonal,
  // which are only available at updateSolver() time. _solver is intentionally left null
  // here; getSolver() must not be called before updateSolver().
}

void
OperatorChebyshevSmoother::updateSolver(mfem::Operator & op, mfem::Array<int> & tdofs)
{
  _diag.SetSize(op.Height());
  op.AssembleDiagonal(_diag);
  _solver = std::make_unique<mfem::OperatorChebyshevSmoother>(
      op, _diag, tdofs, _order, getMFEMProblem().getComm());
}

} // namespace Moose::MFEM
#endif
