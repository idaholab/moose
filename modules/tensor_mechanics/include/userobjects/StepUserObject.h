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
 * User object that provides simulation steps given times of the transient simulation.
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

  // Times at which steps start/end
  const std::vector<Real> _times;

  // Total number of loading steps
  const unsigned int _number_steps;
};
