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

namespace Moose::MFEM
{
/**
 * Base class for LOR compatible linear MFEM solvers and preconditioners.
 * Any LOR-capable MFEM solver T must inherit from LORLinearSolverBase<T>.
 * The user is free to specialize LORLinearSolverBase<T>::UpdateEquationSystemContext()
 * if the provided template definition is not appropriate, for instance if additional
 * setup is required or if building an mfem::LORSolver<T> object is unnecessary.
 */
template <class MFEMSolverType>
class LORLinearSolverBase : public LinearSolverBase
{
public:
  static InputParameters validParams();

  LORLinearSolverBase(const InputParameters & parameters);

protected:
  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(MFEMSolverType & solver) = 0;

  virtual void UpdateEquationSystemContext() override;

  /// Variable defining whether to use LOR solver
  bool _lor;
  mfem::ParBilinearForm * _a;
  mfem::Array<int> _ess_bdr_markers;
  mfem::Array<int> _ess_tdofs;

private:
  void SetLORSolver();

  /// Checks for the correct configuration of quadrature bases for LOR spectral equivalence
  virtual void CheckSpectralEquivalence(mfem::ParBilinearForm & blf) const;

  /// Rebuild any Low-Order-Refined components from the unreduced bilinear form. Called only when
  /// _lor is true, before the assembled linear operator has been set via SetOperator. Default
  /// no-op; override in solvers or preconditioners that construct LOR-related data from the
  /// bilinear form.
  virtual void SetupLOR(std::shared_ptr<Moose::MFEM::EquationSystem> equation_system);
};

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
  : LinearSolverBase(parameters), _lor(parameters.get<bool>("low_order_refined"))
{
}

template <typename T, typename... Ts>
constexpr bool is_any_of_v = (std::is_same_v<T, Ts> || ...);

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
LORLinearSolverBase<MFEMSolverType>::UpdateEquationSystemContext()
{
  Moose::MFEM::LinearSolverBase::UpdateEquationSystemContext();
  if (_lor && GetPreconditioner())
    mooseError("LOR solver cannot take a preconditioner");
  if (_lor)
  {
    SetupLOR(_equation_system);
    if constexpr (is_any_of_v<MFEMSolverType, mfem::HypreAMS, mfem::HypreADS>)
      if (_a->ParFESpace()->GetMesh()->GetTypicalElementGeometry() != mfem::Geometry::Type::CUBE)
        mooseError("LOR HypreAMS/ADS Solver only supports hex meshes.");
    SetLORSolver();
  }
  else if constexpr (!is_any_of_v<MFEMSolverType,
                                  mfem::OperatorJacobiSmoother,
                                  mfem::HypreBoomerAMG,
                                  mfem::HypreAMS,
                                  mfem::HypreADS>)
    SetPreconditioner(static_cast<MFEMSolverType &>(GetSolver()));
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::SetupLOR(
    std::shared_ptr<Moose::MFEM::EquationSystem> equation_system)
{
  if (!equation_system)
    mooseError("LOR solver setup requires an EquationSystem to be defined.");
  if (equation_system->IsComplex())
    mooseError("LOR solve is not supported for complex equation systems.");
  if (equation_system->IsMultivariate())
    mooseError("LOR solve is only supported for single-variable systems");

  const auto & test_var_name = equation_system->GetTestVarNames().at(0);
  _a = &equation_system->GetBilinearForm(test_var_name);
  CheckSpectralEquivalence(*_a);
  _ess_bdr_markers = equation_system->GetEssentialBoundaryMarkers(test_var_name);
  _a->ParFESpace()->GetEssentialTrueDofs(_ess_bdr_markers, _ess_tdofs);
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::SetLORSolver()
{
  mfem::LORSolver<MFEMSolverType> * lor_solver;
  if constexpr (is_any_of_v<MFEMSolverType, mfem::HypreGMRES, mfem::HypreFGMRES, mfem::HyprePCG>)
  {
    mfem::ParLORDiscretization lor_disc(*_a, _ess_tdofs);
    lor_solver = new mfem::LORSolver<MFEMSolverType>(lor_disc, _a->ParFESpace()->GetComm());
  }
  else
    lor_solver = new mfem::LORSolver<MFEMSolverType>(*_a, _ess_tdofs);
  SetSolverParameters(lor_solver->GetSolver());
  SetSolver(lor_solver);
}

} // namespace Moose::MFEM

#endif
