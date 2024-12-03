//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Reports on the number of time steps already taken in the transient
 */
class NumTimeSteps : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NumTimeSteps(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  /**
   * This will return the current time step number.
   */
  virtual Real getValue() const override;

protected:
  FEProblemBase & _feproblem;
};
