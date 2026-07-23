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
