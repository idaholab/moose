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
  : MFEMGeneralUserObject(parameters), _lor{getParam<bool>("low_order_refined")}
{
}

template <typename T>
void
MFEMSolverBase::setPreconditioner(const std::shared_ptr<T> & solver)
{
  static_assert(std::is_base_of_v<mfem::Solver, T>);

  if (isParamSetByUser("preconditioner"))
  {
    if (!_preconditioner)
      _preconditioner =
          &const_cast<MFEMSolverBase &>(getUserObject<MFEMSolverBase>("preconditioner"));

    if constexpr (std::is_base_of_v<mfem::HypreSolver, T>)
      if (auto hypre = std::dynamic_pointer_cast<mfem::HypreSolver>(_preconditioner->getSolver()))
        solver->SetPreconditioner(*hypre);
      else
        mooseError("hypre solver preconditioners must themselves be hypre solvers");
    else if constexpr (std::is_base_of_v<mfem::IterativeSolver, T>)
      solver->SetPreconditioner(*_preconditioner->getSolver());
    else
      mooseError("Cannot set preconditioner for the given MFEM solver");
  }
}

template void MFEMSolverBase::setPreconditioner(const std::shared_ptr<mfem::CGSolver> &);
template void MFEMSolverBase::setPreconditioner(const std::shared_ptr<mfem::GMRESSolver> &);
template void MFEMSolverBase::setPreconditioner(const std::shared_ptr<mfem::HypreFGMRES> &);
template void MFEMSolverBase::setPreconditioner(const std::shared_ptr<mfem::HypreGMRES> &);
template void MFEMSolverBase::setPreconditioner(const std::shared_ptr<mfem::HyprePCG> &);

#endif
