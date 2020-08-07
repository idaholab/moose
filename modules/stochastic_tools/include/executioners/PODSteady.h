//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Steady.h"

class PODSteady : public Steady
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  static InputParameters validParams();

  PODSteady(const InputParameters & parameters);

  virtual void execute() override;

  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:

  PerfID _post_snapshot_timer;

private:
  bool _last_solve_converged;
};
