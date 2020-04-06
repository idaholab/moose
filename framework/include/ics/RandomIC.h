//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RandomICBase.h"
#include "DistributionInterface.h"

// Forward Declarations
class InputParameters;
class Distribution;
namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

/**
 * RandomIC just returns a Random value.
 */
class RandomIC : public RandomICBase, public DistributionInterface
{
public:
  /**
   * Constructor
   * @param parameters The parameters object holding data for the class to use.
   */
  RandomIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

  static InputParameters validParams();

protected:
  /// The lower bound of the random number range
  const Real _min;

  /// The upper bound of the random number range
  const Real _max;

  /// Distribution object optionally used to define distribution of random numbers
  Distribution const * _distribution;
};
