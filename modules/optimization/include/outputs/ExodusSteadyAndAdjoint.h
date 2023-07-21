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
#include "SteadyAndAdjoint.h"

/**
 * Class for output data to the ExodusII format
 */
class ExodusSteadyAndAdjoint : public Exodus
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   */
  ExodusSteadyAndAdjoint(const InputParameters & parameters);

protected:
  /**
   * Outputs nodal, nonlinear variables
   */
  virtual void outputNodalVariables() override;

  /**
   * Increment file counter with steady and adjoint iteration number
   */
  virtual void incrementFileCounter() override;

private:
  /// For steady and adjoint executioner
  const SteadyAndAdjoint * const _steady_and_adjoint_exec;
};
