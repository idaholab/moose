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
#include "MFEMProblemData.h"
#include "ProblemOperatorInterface.h"
#include "MFEMEstimator.h"
#include "MFEMThresholdRefiner.h"

namespace Moose::MFEM
{
/// Steady-state problem operator with no equation system.
class ProblemOperator : public mfem::Operator, public ProblemOperatorInterface
{
public:
  ProblemOperator(MFEMProblemData & problem) : ProblemOperatorInterface(problem) {}

  void SetGridFunctions() override;

  /*
  These functions are for doing AMR stuff with - however we only want this to happen in
  the EquationSystemProblemOperator. Since that one inherits from this class, we need
  to add trivial virtual functions up here to make that possible.
  */
  virtual void AddEstimator(std::shared_ptr<MFEMEstimator> /*estimator*/) {}
  virtual void AddRefiner(std::shared_ptr<MFEMThresholdRefiner> /*refiner*/) {}
  virtual void SetUpAMR() {};
  virtual bool HRefine() { return false; } /* we return true when it's time to stop solving */
  virtual bool PRefine() { return false; } /* we return true when it's time to stop solving */
  virtual bool UseHRefinement() const { return false; }
  virtual bool UsePRefinement() const { return false; }

  virtual void Solve(mfem::Vector & /*X*/) {}
  void Mult(const mfem::Vector &, mfem::Vector &) const override {}
};

} // namespace Moose::MFEM

#endif
