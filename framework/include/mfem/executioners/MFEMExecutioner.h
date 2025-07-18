//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "Executioner.h"
#include "MFEMProblemData.h"
#include "MFEMThresholdMarker.h"

class MFEMProblem;

class MFEMExecutioner : public Executioner
{
public:
  static InputParameters validParams();

  MFEMExecutioner(const InputParameters & params);

  virtual bool lastSolveConverged() const override { return true; };

  /// Virtual method to construct the ProblemOperator. Call for default problems.
  virtual void constructProblemOperator() = 0;

  // Executioners should not support estimators by default
  virtual bool addRefiner(std::shared_ptr<MFEMThresholdMarker>) { return false; }

  virtual void UpdateAfterRefinement() {}

  //! Virtual method to apply any refinements which are enabled and returns a bool
  //! to indicate whether we should solve the problem again.
  virtual bool ApplyRefinements() {return false;}

  bool UseAMR() const;

  /**
   * Set the device to use to solve the FE problem.
   */
  void setDevice();

protected:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _problem_data;
  mfem::Device _device;
};

#endif
