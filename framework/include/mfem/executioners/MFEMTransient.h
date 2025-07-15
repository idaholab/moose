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
#include "MFEMExecutioner.h"
#include "TimeDomainProblemOperator.h"
#include "TransientBase.h"

class MFEMTransient : public TransientBase, public MFEMExecutioner
{
public:
  static InputParameters validParams();

  explicit MFEMTransient(const InputParameters & params);

  void constructProblemOperator() override;
  virtual void init() override;

  /// Do whatever is necessary to advance one step.
  virtual void takeStep(Real input_dt = -1.0) override;

  /// Perform all required solves during a step. Called within takeStep.
  virtual void innerSolve() override;

  /// Check if last solve converged. Currently defaults to true for all MFEM executioners.
  virtual bool lastSolveConverged() const override { return true; };

  /// Not supported for MFEM problems, so error if called.
  virtual Real relativeSolutionDifferenceNorm() override
  {
    mooseError("MFEMTransient executioner does not yet support evaluating the relative solution "
               "difference norm at each timestep.");
    return 0.0;
  };

  /// MFEM problems have no libMesh type TimeIntegrators attached, so return empty set.
  virtual std::set<TimeIntegrator *> getTimeIntegrators() const override
  {
    return std::set<TimeIntegrator *>{};
  };

  /// MFEM problems have no libMesh type TimeIntegrators attached, so return empty vector.
  virtual std::vector<std::string> getTimeIntegratorNames() const override
  {
    return std::vector<std::string>();
  };

private:
  std::unique_ptr<Moose::MFEM::TimeDomainProblemOperator> _problem_operator{nullptr};
};

#endif
