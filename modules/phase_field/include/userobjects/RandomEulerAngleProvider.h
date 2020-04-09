//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EulerAngleProvider.h"
#include "MooseRandom.h"

// Forward declaration
class GrainTrackerInterface;

/**
 * Assign random Euler angles to each grains
 */
class RandomEulerAngleProvider : public EulerAngleProvider
{
public:
  static InputParameters validParams();

  RandomEulerAngleProvider(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

  virtual const EulerAngles & getEulerAngles(unsigned int) const override;
  virtual unsigned int getGrainNum() const override;

protected:
  const GrainTrackerInterface & _grain_tracker;
  std::vector<EulerAngles> _angles;

  MooseRandom _random;
};
