//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "FullSolveMultiApp.h"
#include "SamplerInterface.h"

class SamplerFullSolveMultiApp;
class Sampler;

template <>
InputParameters validParams<SamplerFullSolveMultiApp>();

class SamplerFullSolveMultiApp : public FullSolveMultiApp, public SamplerInterface
{
public:
  SamplerFullSolveMultiApp(const InputParameters & parameters);

  /**
   * Return the Sampler object for this MultiApp.
   */
  Sampler & getSampler() const { return _sampler; }

  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

protected:
  /// Sampler to utilize for creating MultiApps
  Sampler & _sampler;

  /// The Sup-application solve mode
  const MooseEnum & _mode;

private:
  /**Helper method for running in mode='batch'*/
  bool solveStepBatch(Real dt, Real target_time, bool auto_advance = true);
};
