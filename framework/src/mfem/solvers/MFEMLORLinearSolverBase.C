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
  params.addParam<bool>("low_order_refined", false, "Set usage of Low-Order Refined solver.");
  return params;
}

template <class MFEMSolverType>
LORLinearSolverBase<MFEMSolverType>::LORLinearSolverBase(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters), _lor{parameters.get<bool>("low_order_refined")}
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
      LORLinearSolverBase<MFEMSolverType>::SetupLOR(*this, *_equation_system);
  }  
}

template <class MFEMSolverType>
bool
LORLinearSolverBase<MFEMSolverType>::IsLOR(LinearSolverBase & solver) const
{
  LORLinearSolverBase<MFEMSolverType> * lor_preconditioner = GetPreconditionerLORInterface(solver);
  return _lor || (lor_preconditioner && lor_preconditioner->IsLOR(*solver.GetPreconditioner()));
}

template <class MFEMSolverType>
LORLinearSolverBase<MFEMSolverType> *
LORLinearSolverBase<MFEMSolverType>::GetPreconditionerLORInterface(LinearSolverBase & solver) const
{
  return dynamic_cast<LORLinearSolverBase<MFEMSolverType> *>(solver.GetPreconditioner());
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::CheckSpectralEquivalence(mfem::ParBilinearForm & blf) const
{
  if (auto fec = dynamic_cast<const mfem::H1_FECollection *>(blf.FESpace()->FEColl()))
  {
    if (fec->GetBasisType() != mfem::BasisType::GaussLobatto)
      mooseError("Low-Order-Refined solver requires the FESpace basis to be GaussLobatto "
                 "for H1 elements.");
  }
  else if (auto fec = dynamic_cast<const mfem::ND_FECollection *>(blf.FESpace()->FEColl()))
  {
    if (fec->GetClosedBasisType() != mfem::BasisType::GaussLobatto ||
        fec->GetOpenBasisType() != mfem::BasisType::IntegratedGLL)
      mooseError("Low-Order-Refined solver requires the FESpace closed-basis to be GaussLobatto "
                 "and the open-basis to be IntegratedGLL for ND elements.");
  }
  else if (auto fec = dynamic_cast<const mfem::RT_FECollection *>(blf.FESpace()->FEColl()))
  {
    if (fec->GetClosedBasisType() != mfem::BasisType::GaussLobatto ||
        fec->GetOpenBasisType() != mfem::BasisType::IntegratedGLL)
      mooseError("Low-Order-Refined solver requires the FESpace closed-basis to be GaussLobatto "
                 "and the open-basis to be IntegratedGLL for RT elements.");
  }
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::SetupLOR(Moose::MFEM::EquationSystem & equation_system)
{
  if (equation_system.isComplex())
    mooseError("LOR solve is not supported for complex equation systems.");
  if (equation_system.GetTestVarNames().size() > 1)
    mooseError("LOR solve is only supported for single-variable systems");

  const auto & test_var_name = equation_system.GetTestVarNames().at(0);
  const auto & trial_var_name = equation_system.GetTrialVarNames().at(0);
  mfem::ParGridFunction & trial_gf = equation_system.getGridFunction(trial_var_name);
  _a = &equation_system.GetBilinearForm(test_var_name);
  CheckSpectralEquivalence(*_a);

  _ess_bdr_markers.SetSize(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max());
  _ess_bdr_markers = 0;
  equation_system.ApplyEssentialBC(trial_var_name, trial_gf, _ess_bdr_markers);

  _a->ParFESpace()->GetEssentialTrueDofs(_ess_bdr_markers, _ess_tdofs);
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::SetupLOR(LinearSolverBase & solver_base,
                                              Moose::MFEM::EquationSystem & equation_system)
{
  if (_lor)
  {
    SetupLOR(equation_system);
    auto lor_solver = new mfem::LORSolver<MFEMSolverType>(*_a, _ess_tdofs);
    SetSolverParameters(lor_solver->GetSolver());
    solver_base.SetSolver(lor_solver);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreGMRES>::SetupLOR(LinearSolverBase & solver_base,
                                          Moose::MFEM::EquationSystem & equation_system)
{
  if (_lor)
  {
    SetupLOR(equation_system);
    mfem::ParLORDiscretization lor_disc(*_a, _ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HypreGMRES>(lor_disc, _a->ParFESpace()->GetComm());
    SetSolverParameters(lor_solver->GetSolver());
    solver_base.SetSolver(lor_solver);
  }
}

template <>
void
LORLinearSolverBase<mfem::HypreFGMRES>::SetupLOR(LinearSolverBase & solver_base,
                                          Moose::MFEM::EquationSystem & equation_system)
{
  if (_lor)
  {
    SetupLOR(equation_system);    
    mfem::ParLORDiscretization lor_disc(*_a, _ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HypreFGMRES>(lor_disc, _a->ParFESpace()->GetComm());
    SetSolverParameters(lor_solver->GetSolver());
    solver_base.SetSolver(lor_solver);
  }
}

template <>
void
LORLinearSolverBase<mfem::HyprePCG>::SetupLOR(LinearSolverBase & solver_base,
                                       Moose::MFEM::EquationSystem & equation_system)
{
  if (_lor)
  {
    SetupLOR(equation_system);
    mfem::ParLORDiscretization lor_disc(*_a, _ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HyprePCG>(lor_disc, _a->ParFESpace()->GetComm());
    SetSolverParameters(lor_solver->GetSolver());
    solver_base.SetSolver(lor_solver);
  }
}

template <>
void
LORLinearSolverBase<Moose::MFEM::MatrixFreeAMS>::SetupLOR(LinearSolverBase & solver_base,
                                                          Moose::MFEM::EquationSystem & equation_system)
{
  SetupLOR(equation_system);
  auto & matrix_free_ams = static_cast<Moose::MFEM::MatrixFreeAMS &>(*_solver);
  matrix_free_ams.SetBilinearForm(*_a);
  matrix_free_ams.SetBoundaryMarkers(_ess_bdr_markers);
}

} // namespace Moose::MFEM

#endif
