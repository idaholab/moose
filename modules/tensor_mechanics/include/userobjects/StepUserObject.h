//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * User object that provides simulation steps given user input
 */
class StepUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  StepUserObject(const InputParameters & parameters);

  // Get start time
  Real getStartTime(const unsigned int & step) const;
  // Get end time
  Real getEndTime(const unsigned int & step) const;
  // Get step given current time
  unsigned int getStep(const Real & time) const;

protected:
  void initialize() override;
  void execute() override;
  void finalize() override;

  // Step start times
  std::vector<Real> _times;

  // Step durations that define loading steps
  std::vector<Real> _step_durations;

  // Total time interval that's divided into equally sized steps
  Real _total_time_interval;

  // Number of steps to divide the total_time_interval
  unsigned int _number_steps;
};
