//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Times.h"

/**
 * Times simulated, obtained from the problem
 */
class SimulationTimes : public Times
{
public:
  static InputParameters validParams();
  SimulationTimes(const InputParameters & parameters);
  virtual ~SimulationTimes() = default;

protected:
  virtual void initialize() override;
};
