//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMSolverBase.h"

InputParameters
MFEMSolverBase::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.addClassDescription("Base class for defining mfem::Solver derived classes for Moose.");
  params.registerBase("MFEMSolverBase");
  params.addParam<bool>("low_order_refined", false, "Set usage of Low-Order Refined solver.");

  return params;
}

MFEMSolverBase::MFEMSolverBase(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _lor{getParam<bool>("low_order_refined")},
    _solver{nullptr},
    _preconditioner{nullptr}
{
}

template <typename T>
void
MFEMSolverBase::setPreconditioner(T & solver)
{
  if (isParamSetByUser("preconditioner"))
  {
    if (!_preconditioner)
      _preconditioner =
          &const_cast<MFEMSolverBase &>(getUserObject<MFEMSolverBase>("preconditioner"));

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

template void MFEMSolverBase::setPreconditioner(mfem::CGSolver &);
template void MFEMSolverBase::setPreconditioner(mfem::GMRESSolver &);
template void MFEMSolverBase::setPreconditioner(mfem::HypreFGMRES &);
template void MFEMSolverBase::setPreconditioner(mfem::HypreGMRES &);
template void MFEMSolverBase::setPreconditioner(mfem::HyprePCG &);

#endif
