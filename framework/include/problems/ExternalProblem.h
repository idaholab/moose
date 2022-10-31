//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblemBase.h"

class ExternalProblem : public FEProblemBase
{
public:
  static InputParameters validParams();

  ExternalProblem(const InputParameters & parameters);

  enum class Direction : unsigned char
  {
    TO_EXTERNAL_APP,
    FROM_EXTERNAL_APP
  };

  /**
   * Solve is implemented to providing syncing to/from the "transfer" mesh.
   */
  virtual void solve(unsigned int nl_sys_num = 0) override final;

  /**
   * New interface for solving an External problem. "solve()" is finalized here to provide
   * callbacks for solution syncing.
   */
  virtual void externalSolve() = 0;

  /**
   * Method to transfer data to/from the external application to the associated transfer mesh.
   */
  virtual void syncSolutions(Direction direction) = 0;

  /**
   * Method called to add AuxVariables to the simulation. These variables would be the fields
   * that should either be saved out with the MOOSE-formatted solutions or available for
   * transfer to variables in Multiapp simulations.
   */
  virtual void addExternalVariables() {}
};
