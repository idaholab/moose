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

#include "ProblemOperator.h"

/**
 * Custom Dummy Operator with a basic solve
 * uses same problem as mfem ex0p
 */
class CustomDummyProblemOperator : public Moose::MFEM::ProblemOperator
{
private:
  // The linear and bilinear forms
  mfem::ParBilinearForm * _a;
  mfem::ParLinearForm * _b;

  // The coefficient
  mfem::ConstantCoefficient _one;

  // The boundary conditions arrays
  mfem::Array<int> _boundary_dofs;

  // The operator and solution vectors (could
  // potentially use the ones in the base class)
  mfem::OperatorHandle _problem_operator; // The actual mfem problem operator
  mfem::Vector _B, _X;

public:
  // The constructor
  CustomDummyProblemOperator(MFEMProblem & prob_ex0p);

  // The destructor
  ~CustomDummyProblemOperator() = default;

  // The initialisation function
  virtual void Init(mfem::BlockVector &) override;

  // Solve the equation
  virtual void Solve() override;

  // Apply the operator
  void Mult(const mfem::Vector &, mfem::Vector &) const override;
};

#endif
