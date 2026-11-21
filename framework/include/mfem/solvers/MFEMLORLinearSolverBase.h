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
 * An mfem::LORSolver<T> that keeps the low-order-refined operator it was built with.
 *
 * mfem::LORSolver<T>::SetOperator() passes the operator on to the wrapped solver, which then
 * solves with it instead of the LOR operator. A Krylov solver calls SetOperator() on its
 * preconditioner with the operator it solves, which is the high-order one, so an LOR
 * preconditioner would silently stop being LOR; and if the high-order operator is not a
 * HypreParMatrix, as with partial assembly, a hypre preconditioner fails. Here SetOperator() only
 * checks the size of the operator, and the LOR operator is rebuilt from the bilinear form by
 * LORLinearSolverBase instead.
 */
template <class MFEMSolverType>
class LORSolver : public mfem::LORSolver<MFEMSolverType>
{
public:
  using mfem::LORSolver<MFEMSolverType>::LORSolver;

  /// Build the solver on the LOR operator of @a lor, taking ownership of @a lor. @a args are
  /// passed to the constructor of the wrapped solver.
  template <typename... Args>
  LORSolver(std::unique_ptr<mfem::ParLORDiscretization> lor, Args &&... args)
    : mfem::LORSolver<MFEMSolverType>(*lor, std::forward<Args>(args)...)
  {
    this->own_lor = true;
    lor.release();
  }

  /// Keeps the LOR operator; @a op only has to match its size
  void SetOperator(const mfem::Operator & op) override
  {
    if (op.Height() != this->Height() || op.Width() != this->Width())
      mooseError("The operator given to a low-order-refined solver has size ",
                 op.Height(),
                 " x ",
                 op.Width(),
                 ", but its LOR operator has size ",
                 this->Height(),
                 " x ",
                 this->Width(),
                 ".");
  }
};

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
  virtual void SetupLOR();
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
  if (!_lor)
    return;

  if (isParamSetByUser("preconditioner"))
    paramError("preconditioner", "LOR solver cannot take a preconditioner");

  SetupLOR();

  if constexpr (is_any_of_v<MFEMSolverType, mfem::HypreAMS, mfem::HypreADS>)
    if (_a->ParFESpace()->GetMesh()->GetTypicalElementGeometry() != mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreAMS/ADS Solver only supports hex meshes.");

  SetLORSolver();
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::SetupLOR()
{
  if (!_equation_system)
    mooseError("LOR solver setup requires an EquationSystem to be defined.");
  if (_equation_system->IsComplex())
    mooseError("LOR solve is not supported for complex equation systems.");
  if (_equation_system->IsMultivariate())
    mooseError("LOR solve is only supported for single-variable systems");

  const auto & test_var_name = _equation_system->GetTestVarNames().at(0);
  _a = &_equation_system->GetBilinearForm(test_var_name);
  CheckSpectralEquivalence(*_a);
  _ess_bdr_markers = _equation_system->GetEssentialBoundaryMarkers(test_var_name);
  _a->ParFESpace()->GetEssentialTrueDofs(_ess_bdr_markers, _ess_tdofs);
}

template <class MFEMSolverType>
void
LORLinearSolverBase<MFEMSolverType>::SetLORSolver()
{
  LORSolver<MFEMSolverType> * lor_solver;
  // Krylov solvers are given the communicator, without which mfem::CGSolver and
  // mfem::GMRESSolver compute rank-local inner products, and the hypre ones cannot be built
  if constexpr (is_any_of_v<MFEMSolverType,
                            mfem::HypreGMRES,
                            mfem::HypreFGMRES,
                            mfem::HyprePCG,
                            mfem::CGSolver,
                            mfem::GMRESSolver>)
    lor_solver = new LORSolver<MFEMSolverType>(
        std::make_unique<mfem::ParLORDiscretization>(*_a, _ess_tdofs), _a->ParFESpace()->GetComm());
  else
    lor_solver = new LORSolver<MFEMSolverType>(*_a, _ess_tdofs);
  SetSolverParameters(lor_solver->GetSolver());
  SetSolver(lor_solver);
}

} // namespace Moose::MFEM

#endif
