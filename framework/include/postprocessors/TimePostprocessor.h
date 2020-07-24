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
 * Postprocessor that returns the current time
 */
class TimePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TimePostprocessor(const InputParameters & parameters);

  void initialize() override {}
  void execute() override {}

  Real getValue() override;

protected:
  const FEProblemBase & _feproblem;
};
