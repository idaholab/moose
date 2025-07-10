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

  /**
   * Do whatever is necessary to advance one step.
   */
  virtual void takeStep(Real input_dt = -1.0) override;

  virtual bool lastSolveConverged() const override { return true; };

  virtual Real relativeSolutionDifferenceNorm() override { return 0.0; };

  /**
   * Get the time integrators (time integration scheme) used
   * Note that because some systems might be steady state simulations, there could be less
   * time integrators than systems
   * @return string with the time integration scheme name
   */
  virtual std::set<TimeIntegrator *> getTimeIntegrators() const override
  {
    return std::set<TimeIntegrator *>{};
  };

  /**
   * Get the name of the time integrator (time integration scheme) used
   * @return string with the time integration scheme name
   */
  virtual std::vector<std::string> getTimeIntegratorNames() const override
  {
    return std::vector<std::string>();
  };

private:
  int _vis_steps;          // Number of cycles between each output update
  mutable bool _last_step; // Flag to check if current step is final
  std::unique_ptr<Moose::MFEM::TimeDomainProblemOperator> _problem_operator{nullptr};
};

#endif
