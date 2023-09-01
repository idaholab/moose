//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Times.h"

/**
 * Times between a start time and end time with a fixed time interval.
 */
class TimeIntervalTimes : public Times
{
public:
  static InputParameters validParams();
  TimeIntervalTimes(const InputParameters & parameters);
  virtual ~TimeIntervalTimes() = default;

protected:
  virtual void initialize() override {}
};
