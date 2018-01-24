//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSTANTIC_H
#define CONSTANTIC_H

#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class ConstantIC;
class InputParameters;

namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<ConstantIC>();

/**
 * ConstantIC just returns a constant value.
 */
class ConstantIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  ConstantIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  Real _value;
};

#endif // CONSTANTIC_H
