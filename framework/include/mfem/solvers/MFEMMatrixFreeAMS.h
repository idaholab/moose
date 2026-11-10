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
 * Wrapper for mfem::MatrixFreeAMS solver.
 */
class MFEMMatrixFreeAMS : public Moose::MFEM::LORLinearSolverBase<mfem::MatrixFreeAMS>
{
public:
  static InputParameters validParams();

  MFEMMatrixFreeAMS(const InputParameters &);

  void ConstructSolver() override;

protected:
  // Constructs the mfem::MatrixFreeAMS solver for the given operator.
  virtual void SetOperatorImpl(const mfem::Operator & op) override;
  virtual void SetSolverParameters(mfem::MatrixFreeAMS &) override {}

private:
  mfem::Coefficient & _alpha_coef;
  mfem::Coefficient & _beta_coef;
  const int _inner_pi_its;
  const int _inner_g_its;
};

#endif
