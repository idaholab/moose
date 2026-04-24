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

registerMooseObject("MooseApp", MFEMOperatorChebyshevSmoother);

InputParameters
MFEMOperatorChebyshevSmoother::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription(
      "Chebyshev polynomial smoother backed by mfem::OperatorChebyshevSmoother. "
      "Symmetric positive definite by construction; the maximum eigenvalue of D^{-1}A "
      "is estimated automatically via a power method. Suitable as a multigrid smoother "
      "without requiring a manual damping parameter.");
  params.addParam<int>("order", 2, "Degree of the Chebyshev polynomial (1–4).");
  return params;
}

MFEMOperatorChebyshevSmoother::MFEMOperatorChebyshevSmoother(const InputParameters & parameters)
  : LinearSolverBase(parameters), _order(getParam<int>("order"))
{
  ConstructSolver();
}

void
MFEMOperatorChebyshevSmoother::ConstructSolver()
{
  // Deferred: mfem::OperatorChebyshevSmoother requires the operator and its diagonal,
  // which are only available at SetOperator() time. _solver is intentionally left null
  // here; GetSolver() must not be called before SetOperator().
}

void
MFEMOperatorChebyshevSmoother::SetOperator(mfem::Operator & op)
{
  _diag.SetSize(op.Height());
  op.AssembleDiagonal(_diag);
  _solver = std::make_unique<mfem::OperatorChebyshevSmoother>(
      op, _diag, _empty_tdofs, _order, getMFEMProblem().getComm());
}
#endif
