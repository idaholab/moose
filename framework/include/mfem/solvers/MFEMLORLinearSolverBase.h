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
  void SetLORSolver();
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

template <typename T, typename... Ts>
constexpr bool is_any_of_v = (std::is_same_v<T, Ts> || ...);

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
      if constexpr (is_any_of_v<MFEMSolverType, mfem::HypreAMS, mfem::HypreADS>)
        if (_a->ParFESpace()->GetMesh()->GetElement(0)->GetGeometryType() !=
            mfem::Geometry::Type::CUBE)
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
