//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMLinearSolverBase.h"
#include "MFEMLORInterface.h"

namespace Moose::MFEM
{
/**
 * Base class for LOR compatible linear MFEM solvers and preconditioners.
 */
template <class MFEMSolverType>
class LORLinearSolverBase : public LinearSolverBase, public LORInterface
{
public:
  static InputParameters validParams();

  LORLinearSolverBase(const InputParameters & parameters);

protected:
  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(MFEMSolverType & solver) = 0;

  virtual void UpdateEquationSystemContext() override;

private:
  void SetLORSolver(LinearSolverBase & solver);
};

template <class MFEMSolverType>
InputParameters
LORLinearSolverBase<MFEMSolverType>::validParams()
{
  InputParameters params = LinearSolverBase::validParams();
  params += LORInterface::validParams();
  return params;
}

template <class MFEMSolverType>
LORLinearSolverBase<MFEMSolverType>::LORLinearSolverBase(const InputParameters & parameters)
  : LinearSolverBase(parameters), LORInterface(parameters)
{
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::UpdateEquationSystemContext()
{
  Moose::MFEM::LinearSolverBase::UpdateEquationSystemContext();
  if (IsLOR(*this))
  {
    if (_lor && GetPreconditioner())
      mooseError("LOR solver cannot take a preconditioner");
    if (_lor)
    {
      SetupLOR(_equation_system);
      LORLinearSolverBase<MFEMSolverType>::SetLORSolver(*this);
    }
    else
      SetPreconditioner(static_cast<MFEMSolverType &>(GetSolver()));
  }
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::SetLORSolver(LinearSolverBase & solver_base)
{
  if (_lor)
  {
    auto lor_solver = new mfem::LORSolver<MFEMSolverType>(*_a, _ess_tdofs);
    SetSolverParameters(lor_solver->GetSolver());
    solver_base.SetSolver(lor_solver);
  }
}

// Template specializations required for context updates for solvers that cannot take
// preconditioners
template <>
void LORLinearSolverBase<mfem::OperatorJacobiSmoother>::UpdateEquationSystemContext();

template <>
void LORLinearSolverBase<mfem::HypreBoomerAMG>::UpdateEquationSystemContext();

template <>
void LORLinearSolverBase<mfem::HypreAMS>::UpdateEquationSystemContext();

template <>
void LORLinearSolverBase<mfem::HypreADS>::UpdateEquationSystemContext();

// Template specializations required for LOR wrappers for Hypre iterative solvers that lack default
// constructors
template <>
void LORLinearSolverBase<mfem::HypreGMRES>::SetLORSolver(LinearSolverBase & solver_base);

template <>
void LORLinearSolverBase<mfem::HypreFGMRES>::SetLORSolver(LinearSolverBase & solver_base);

template <>
void LORLinearSolverBase<mfem::HyprePCG>::SetLORSolver(LinearSolverBase & solver_base);

} // namespace Moose::MFEM

#endif
