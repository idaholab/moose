#pragma once

#include "ScalarInitialCondition.h"

class ScalarSolutionInitialCondition;
class SolutionUserObject;

template <>
InputParameters validParams<ScalarSolutionInitialCondition>();

/**
 * Class for reading an initial condition from a solution user object
 */
class ScalarSolutionInitialCondition : public ScalarInitialCondition
{
public:
  ScalarSolutionInitialCondition(const InputParameters & parameters);

  virtual Real value() override;

protected:
  /// SolutionUserObject containing the solution of interest
  const SolutionUserObject & _solution_object;

  /// The variable name extracted from the SolutionUserObject
  const VariableName & _solution_object_var_name;
};
