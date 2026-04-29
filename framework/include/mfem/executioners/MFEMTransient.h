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

#include "ProblemOperatorInterface.h"
#include "MFEMProblemSolve.h"
#include "TimeDependentProblemOperator.h"
#include "TransientBase.h"

namespace Moose::MFEM
{
class Transient : public TransientBase, public ProblemOperatorInterface
{
public:
  static InputParameters validParams();

  explicit Transient(const InputParameters & params);

  virtual void init() override;

  /// Return the solve object wrapped by time stepper
  virtual SolveObject * timeStepSolveObject() override { return _fixed_point_solve.get(); }

  /// Do whatever is necessary to advance one step.
  virtual void takeStep(Real input_dt = -1.0) override;

  /// Not supported for MFEM problems, so error if called.
  virtual Real relativeSolutionDifferenceNorm(bool /*check_aux*/) const override
  {
    mooseError(
        "Moose::MFEM::Transient executioner does not yet support evaluating the relative solution "
        "difference norm at each timestep.");
    return 0.0;
  }

  /// MFEM problems have no libMesh based TimeIntegrators attached, so return empty set.
  virtual std::set<TimeIntegrator *> getTimeIntegrators() const override { return {}; }

  /// MFEM problems have no libMesh based TimeIntegrators attached, so return empty vector.
  virtual std::vector<std::string> getTimeIntegratorNames() const override { return {}; }

private:
  Problem & _mfem_problem;
  ProblemData & _mfem_problem_data;
  ProblemSolve _mfem_problem_solve;
};

} // namespace Moose::MFEM
#endif
