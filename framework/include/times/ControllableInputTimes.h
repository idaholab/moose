//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "InputTimes.h"

/**
 * Times from an input parameter and from a value that is modified by Controls
 */
class ControllableInputTimes : public InputTimes
{
public:
  static InputParameters validParams();
  ControllableInputTimes(const InputParameters & parameters);
  virtual ~ControllableInputTimes() = default;

protected:
  virtual void initialize() override;

private:
  /// The controllable time attribute
  const Real & _next_time;
};
