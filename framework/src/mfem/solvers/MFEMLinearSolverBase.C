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
  params += LORInterface::validParams();
  params.addClassDescription(
      "Base class for defining linear mfem::Solver derived classes for Moose.");
  return params;
}

LinearSolverBase::LinearSolverBase(const InputParameters & parameters)
  : SolverBase(parameters),
    LORInterface(*this),
    _preconditioner{nullptr},
    _equation_system(getMFEMProblem().getEquationSystem())
{
}

void LinearSolverBase::Update()
{
  if (IsLOR())
    SetupLOR();
}

template <typename T>
void
LinearSolverBase::SetPreconditioner(T & solver)
{
  if (isParamSetByUser("preconditioner"))
  {
    if (!_preconditioner)
    {
      auto & pre = getMFEMProblem().getMFEMObject<LinearSolverBase>(
          "Moose::MFEM::SolverBase", getParam<MFEMSolverName>("preconditioner"));
      // Take shared ownership so the preconditioner outlives the solver
      _preconditioner = std::static_pointer_cast<LinearSolverBase>(pre.shared_from_this());
    }

    if (dynamic_cast<const EigensolverBase *>(GetPreconditioner()))
      mooseError("Eigensolvers cannot be used as preconditioners.");

    auto & mfem_pre = _preconditioner->GetSolver();
    if constexpr (std::is_base_of_v<mfem::HypreSolver, T> || std::is_same_v<mfem::HypreAME, T>)
      if (auto * const hypre_pre = dynamic_cast<mfem::HypreSolver *>(&mfem_pre))
        solver.SetPreconditioner(*hypre_pre);
      else
        mooseError("hypre solver preconditioners must themselves be hypre solvers");
    else
      solver.SetPreconditioner(mfem_pre);
  }
}

template void LinearSolverBase::SetPreconditioner(mfem::CGSolver &);
template void LinearSolverBase::SetPreconditioner(mfem::GMRESSolver &);
template void LinearSolverBase::SetPreconditioner(mfem::HypreFGMRES &);
template void LinearSolverBase::SetPreconditioner(mfem::HypreGMRES &);
template void LinearSolverBase::SetPreconditioner(mfem::HyprePCG &);
template void LinearSolverBase::SetPreconditioner(mfem::HypreLOBPCG &);
template void LinearSolverBase::SetPreconditioner(mfem::HypreAME &);
} // namespace Moose::MFEM

#endif
