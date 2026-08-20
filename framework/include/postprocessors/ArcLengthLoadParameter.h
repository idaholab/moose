//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class ArcLengthProblem;

/**
 * Postprocessor that reports the load parameter lambda of the arc-length continuation driven by
 * ArcLengthProblem
 */
class ArcLengthLoadParameter : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ArcLengthLoadParameter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  /**
   * @return The load parameter cached by the arc-length problem
   */
  virtual Real getValue() const override;

private:
  /// The arc-length problem that owns the reported load parameter
  const ArcLengthProblem * const _arc_length_problem;
};
