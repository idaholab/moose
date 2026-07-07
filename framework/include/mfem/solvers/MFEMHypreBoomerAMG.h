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

#include "MFEMLORLinearSolverBase.h"

/**
 * Wrapper for mfem::HypreBoomerAMG solver.
 */
class MFEMHypreBoomerAMG : public Moose::MFEM::LORLinearSolverBase<mfem::HypreBoomerAMG>
{
public:
  static InputParameters validParams();

  MFEMHypreBoomerAMG(const InputParameters &);
  ~MFEMHypreBoomerAMG();

  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::HypreBoomerAMG & solver) override;

  void Update() override;

protected:
  void ConstructSolver() override;

private:
  std::shared_ptr<mfem::ParFiniteElementSpace> _mfem_fespace{nullptr};
};

#endif
