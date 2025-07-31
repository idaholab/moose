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
#include "MFEMProblemOperatorInterface.h"
#include "MFEMProblemSolve.h"
#include "TimeDomainProblemOperator.h"
#include "TransientBase.h"

class MFEMTransient : public TransientBase, public Moose::MFEM::MFEMProblemOperatorInterface
{
public:
  static InputParameters validParams();

  explicit MFEMTransient(const InputParameters & params);

  virtual void init() override;

  /// Return the solve object wrapped by time stepper
  virtual SolveObject * timeStepSolveObject() override { return &_mfem_problem_solve; }

  /// Do whatever is necessary to advance one step.
  virtual void takeStep(Real input_dt = -1.0) override;

  /// Not supported for MFEM problems, so error if called.
  virtual Real relativeSolutionDifferenceNorm(bool /*check_aux*/) const override
  {
    mooseError("MFEMTransient executioner does not yet support evaluating the relative solution "
               "difference norm at each timestep.");
    return 0.0;
  }

  /// MFEM problems have no libMesh based TimeIntegrators attached, so return empty set.
  virtual std::set<TimeIntegrator *> getTimeIntegrators() const override
  {
    return {};
  }

  /// MFEM problems have no libMesh based TimeIntegrators attached, so return empty vector.
  virtual std::vector<std::string> getTimeIntegratorNames() const override
  {
    return {};
  }

private:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _mfem_problem_data;
  MFEMProblemSolve _mfem_problem_solve;
};

#endif
