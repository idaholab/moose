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
#include "TimesReporter.h"

/**
 * Simple times from an input parameter
 */
class InputTimes : public TimesReporter
{
public:
  static InputParameters validParams();
  InputTimes(const InputParameters & parameters);
  virtual ~InputTimes() = default;

protected:
  virtual void initialize() override {}
};
