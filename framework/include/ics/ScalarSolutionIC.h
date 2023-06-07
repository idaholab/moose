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

class SolutionUserObject;

/**
 * Class for reading an initial condition from a solution user object
 */
class ScalarSolutionIC : public ScalarInitialCondition
{
public:
  ScalarSolutionIC(const InputParameters & parameters);

  virtual Real value() override;

protected:
  /// SolutionUserObject containing the solution of interest
  const SolutionUserObject & _solution_object;

  /// The variable name extracted from the SolutionUserObject
  const VariableName & _solution_object_var_name;

public:
  static InputParameters validParams();
};
