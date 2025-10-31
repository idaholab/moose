//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowChannel1PhaseBase.h"

/**
 * Single-component, single-phase flow channel
 */
class FlowChannel1Phase : public FlowChannel1PhaseBase
{
public:
  static InputParameters validParams();

  FlowChannel1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;
  virtual const THM::FlowModelID & getFlowModelID() const override { return THM::FM_SINGLE_PHASE; }
  virtual std::vector<std::string> ICParameters() const override;
  virtual Convergence * getNonlinearConvergence() const override;

protected:
  virtual void check() const override;
  virtual void checkFluidProperties() const override;
  virtual std::string flowModelClassName() const override;

  /// Checks the residual scaling factors
  void checkScalingFactors() const;
  /// Adds the nonlinear convergence object
  void addNonlinearConvergence();

  /// Nonlinear convergence name
  const std::string _nl_conv_name;
};
