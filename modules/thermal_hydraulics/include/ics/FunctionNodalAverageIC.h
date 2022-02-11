//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * Initial conditions for an elemental variable from a function using nodal average.
 *
 * This IC is typically used when a variable exists both as an elemental variable
 * and a nodal variable, where the following relation is desired to hold:
 * \f[
 *   y^{elem}_0 = \frac{1}{N_{node}}\sum\limits_i^{N_{node}} y_0(x_i) ,
 * \f]
 * where \f$y_0(x)\f$ is the initial condition function,
 * \f$y^{elem}_0\f$ is the computed elemental value,
 * \f$x_i\f$ is the location of node \f$i\f$, and
 * \f$N_{node}\f$ is the number of nodes for the element.
 */
class FunctionNodalAverageIC : public InitialCondition
{
public:
  FunctionNodalAverageIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p) override;

  /// function
  const Function & _func;

public:
  static InputParameters validParams();
};
