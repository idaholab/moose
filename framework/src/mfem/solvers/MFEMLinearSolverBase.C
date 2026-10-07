//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMLinearSolverBase.h"
#include "MFEMProblem.h"
#include "MFEMEigensolverBase.h"

namespace Moose::MFEM
{
InputParameters
LinearSolverBase::validParams()
{
  InputParameters params = SolverBase::validParams();
  params.addClassDescription(
      "Base class for defining linear mfem::Solver derived classes for Moose.");
  params.addParam<MFEMWeakFormName>(
      "weak_form",
      "",
      "Name of the weak form in the WeakForms block whose equation system provides context for "
      "this solver. May be omitted only if the problem has a single weak form, whose equation "
      "system is then used.");
  return params;
}

LinearSolverBase::LinearSolverBase(const InputParameters & parameters)
  : SolverBase(parameters), _preconditioner{nullptr}
{
  const auto & weak_form_name = getParam<MFEMWeakFormName>("weak_form");
  if (!weak_form_name.empty() && !getMFEMProblem().getProblemData().eqn_systems.Has(weak_form_name))
    paramError("weak_form",
               "No weak form named '",
               weak_form_name,
               "' has been added to the problem. Weak forms are added in the 'WeakForms' block.");

  _equation_system = getMFEMProblem().getEquationSystem(weak_form_name);
}

void
LinearSolverBase::SetPreconditioner(const mfem::Operator & op)
{
  if (!isParamSetByUser("preconditioner"))
    return;

  auto & pre = getMFEMProblem().getMFEMObject<LinearSolverBase>(
      "Moose::MFEM::SolverBase", getParam<MFEMSolverName>("preconditioner"));
  if (dynamic_cast<const EigensolverBase *>(&pre))
    paramError("preconditioner", "Eigensolvers cannot be used as preconditioners.");
  // Take shared ownership so the preconditioner outlives the solver
  _preconditioner = std::static_pointer_cast<LinearSolverBase>(pre.getSharedPtr());
  // SetOperatorImpl() may build the preconditioner (e.g. MatrixFreeAMS) for SetPreconditionerImpl()
  _preconditioner->SetOperator(op);
  SetPreconditionerImpl();
}

void
LinearSolverBase::SetOperator(const mfem::Operator & op)
{
  // May replace the wrapped solver (e.g. LOR), so this must precede SetPreconditioner()
  UpdateEquationSystemContext();

  SetPreconditioner(op);

  // SetOperatorImpl() calls SetOperator() on the wrapped mfem solver. In most cases, this will
  // redundantly reset the operator on the preconditioner. For others it requires the preconditioner
  // be already set (e.g. AME), thus why it follows SetPreconditioner(). Repeating SetOperator() for
  // BoomerAMG when used as a preconditioner will also reset it to one V-cycle as expected.
  SetOperatorImpl(op);
}
} // namespace Moose::MFEM

#endif
