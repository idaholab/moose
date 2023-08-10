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
#include "Exodus.h"
#include "Steady.h"

/**
 * Class for output data to the ExodusII format
 */
class ExodusOptimizationSteady : public Exodus
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   */
  ExodusOptimizationSteady(const InputParameters & parameters);

protected:
  /**
   * Customize file output with steady and adjoint iteration number
   */
  void customizeFileOutput() override;

  /**
   * Get time for output (i.e. iteration number for optimization solves).
   */
  virtual Real getOutputTime() override;

private:
  /// For steady (and steady and adjoint) executioner
  const Steady * const _steady_exec;
};
