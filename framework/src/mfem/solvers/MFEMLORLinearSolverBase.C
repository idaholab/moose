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
#include "MFEMMatrixFreeAMS.h"

namespace Moose::MFEM
{
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
LORLinearSolverBase<MFEMSolverType>::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  if (IsLOR(*this))
  {
    if (_lor && GetPreconditioner())
      mooseError("LOR solver cannot take a preconditioner");
    if (_lor)
    {
      SetupLOR(*_equation_system);
      LORLinearSolverBase<MFEMSolverType>::SetLORSolver(*this);
    }
    else
      SetPreconditioner(static_cast<MFEMSolverType &>(GetSolver()));
  }  
}

template <>
void
LORLinearSolverBase<mfem::OperatorJacobiSmoother>::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  if (_lor)
  {
    SetupLOR(*_equation_system);
    LORLinearSolverBase<mfem::OperatorJacobiSmoother>::SetLORSolver(*this);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreBoomerAMG>::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  if (_lor)
  {
    SetupLOR(*_equation_system);
    LORLinearSolverBase<mfem::HypreBoomerAMG>::SetLORSolver(*this);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreAMS>::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  if (_lor)
  {
    SetupLOR(*_equation_system);
    if (_a->ParFESpace()->GetMesh()->GetElement(0)->GetGeometryType() != mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreAMS Solver only supports hex meshes.");
    LORLinearSolverBase<mfem::HypreAMS>::SetLORSolver(*this);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreADS>::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  if (_lor)
  {
    SetupLOR(*_equation_system);
    if (_a->ParFESpace()->GetMesh()->GetElement(0)->GetGeometryType() != mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreADS Solver only supports hex meshes.");
    LORLinearSolverBase<mfem::HypreADS>::SetLORSolver(*this);
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
