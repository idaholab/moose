//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANDOMIC_H
#define RANDOMIC_H

#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class InputParameters;
class RandomIC;
namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<RandomIC>();

/**
 * RandomIC just returns a Random value.
 */
class RandomIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  RandomIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  Real _min;
  Real _max;
  Real _range;
};

#endif // RANDOMIC_H
