//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarInitialCondition.h"

/**
 * Initial condition to set different values on each component of scalar variable
 */
class ScalarComponentIC : public ScalarInitialCondition
{
public:
  static InputParameters validParams();

  ScalarComponentIC(const InputParameters & parameters);

protected:
  virtual Real value();

  std::vector<Real> _initial_values;
};
