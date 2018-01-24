//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONIC_H
#define FUNCTIONIC_H

#include "InitialCondition.h"

#include <string>

// Forward Declarations
class FunctionIC;
class Function;
class InputParameters;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<FunctionIC>();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
class FunctionIC : public InitialCondition
{
public:
  FunctionIC(const InputParameters & parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  Real f();

  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point & p) override;

  /**
   * The value of the gradient at a point.
   */
  virtual RealGradient gradient(const Point & p) override;

  Function & _func;
};

#endif // FUNCTIONIC_H
