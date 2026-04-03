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
#include "MFEMHyprePatch.h"

namespace Moose::MFEM
{
InputParameters
LinearSolverBase::validParams()
{
  InputParameters params = SolverBase::validParams();
  params.addClassDescription(
      "Base class for defining linear mfem::Solver derived classes for Moose.");
  params.addParam<bool>("low_order_refined", false, "Set usage of Low-Order Refined solver.");
  return params;
}

LinearSolverBase::LinearSolverBase(const InputParameters & parameters)
  : SolverBase(parameters), _lor{getParam<bool>("low_order_refined")}, _preconditioner{nullptr}
{
}

template <typename T>
void
LinearSolverBase::setPreconditioner(T & solver)
{
  if (isParamSetByUser("preconditioner"))
  {
    if (!_preconditioner)
      _preconditioner =
          &const_cast<LinearSolverBase &>(getUserObject<LinearSolverBase>("preconditioner"));

    auto & mfem_pre = _preconditioner->getSolver();
    if constexpr (std::is_base_of_v<mfem::HypreSolver, T>)
      if (auto * const hypre_pre = dynamic_cast<mfem::HypreSolver *>(&mfem_pre))
        solver.SetPreconditioner(*hypre_pre);
      else
        mooseError("hypre solver preconditioners must themselves be hypre solvers");
    else
      solver.SetPreconditioner(mfem_pre);
  }
}

template void LinearSolverBase::setPreconditioner(mfem::CGSolver &);
template void LinearSolverBase::setPreconditioner(mfem::GMRESSolver &);
template void LinearSolverBase::setPreconditioner(mfem::HypreFGMRES &);
template void LinearSolverBase::setPreconditioner(mfem::HypreGMRES &);
template void LinearSolverBase::setPreconditioner(mfem::HyprePCG &);

template void LinearSolverBase::setPreconditioner(mfem::patched::HypreGMRES &);
template void LinearSolverBase::setPreconditioner(mfem::patched::HyprePCG &);

void
LinearSolverBase::checkSpectralEquivalence(mfem::ParBilinearForm & blf) const
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
} // namespace Moose::MFEM

#endif
