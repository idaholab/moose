//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "LayeredSideIntegral.h"

/**
 * This UserObject computes side averages of a variable storing partial sums for the specified
 * number of intervals in a direction (x,y,z).
 */
class LayeredSideAverage : public LayeredSideIntegral
{
public:
  static InputParameters validParams();

  LayeredSideAverage(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Value of the volume for each layer
  std::vector<Real> _layer_volumes;
};
