//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMLORLinearSolverBase.h"
#include "MFEMProblem.h"

namespace Moose::MFEM
{
template <>
void
LORLinearSolverBase<mfem::OperatorJacobiSmoother>::UpdateEquationSystemContext()
{
  Moose::MFEM::LinearSolverBase::UpdateEquationSystemContext();
  if (_lor)
  {
    SetupLOR(_equation_system);
    LORLinearSolverBase<mfem::OperatorJacobiSmoother>::SetLORSolver(*this);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreBoomerAMG>::UpdateEquationSystemContext()
{
  Moose::MFEM::LinearSolverBase::UpdateEquationSystemContext();
  if (_lor)
  {
    SetupLOR(_equation_system);
    LORLinearSolverBase<mfem::HypreBoomerAMG>::SetLORSolver(*this);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreAMS>::UpdateEquationSystemContext()
{
  Moose::MFEM::LinearSolverBase::UpdateEquationSystemContext();
  if (_lor)
  {
    SetupLOR(_equation_system);
    if (_a->ParFESpace()->GetMesh()->GetElement(0)->GetGeometryType() != mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreAMS Solver only supports hex meshes.");
    LORLinearSolverBase<mfem::HypreAMS>::SetLORSolver(*this);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreADS>::UpdateEquationSystemContext()
{
  Moose::MFEM::LinearSolverBase::UpdateEquationSystemContext();
  if (_lor)
  {
    SetupLOR(_equation_system);
    if (_a->ParFESpace()->GetMesh()->GetElement(0)->GetGeometryType() != mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreADS Solver only supports hex meshes.");
    LORLinearSolverBase<mfem::HypreADS>::SetLORSolver(*this);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreGMRES>::SetLORSolver(LinearSolverBase & solver_base)
{
  if (_lor)
  {
    mfem::ParLORDiscretization lor_disc(*_a, _ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HypreGMRES>(lor_disc, _a->ParFESpace()->GetComm());
    SetSolverParameters(lor_solver->GetSolver());
    solver_base.SetSolver(lor_solver);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreFGMRES>::SetLORSolver(LinearSolverBase & solver_base)
{
  if (_lor)
  {
    mfem::ParLORDiscretization lor_disc(*_a, _ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HypreFGMRES>(lor_disc, _a->ParFESpace()->GetComm());
    SetSolverParameters(lor_solver->GetSolver());
    solver_base.SetSolver(lor_solver);
  }
}

template <>
void
LORLinearSolverBase<mfem::HyprePCG>::SetLORSolver(LinearSolverBase & solver_base)
{
  if (_lor)
  {
    mfem::ParLORDiscretization lor_disc(*_a, _ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HyprePCG>(lor_disc, _a->ParFESpace()->GetComm());
    SetSolverParameters(lor_solver->GetSolver());
    solver_base.SetSolver(lor_solver);
  }
}

} // namespace Moose::MFEM

#endif
