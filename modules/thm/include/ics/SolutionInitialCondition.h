#pragma once

#include "InitialCondition.h"

class SolutionInitialCondition;
class SolutionUserObject;

template <>
InputParameters validParams<SolutionInitialCondition>();

/**
 * Class for reading an initial condition from a solution user object
 */
class SolutionInitialCondition : public InitialCondition
{
public:
  SolutionInitialCondition(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// SolutionUserObject containing the solution of interest
  const SolutionUserObject & _solution_object;

  /// The variable name extracted from the SolutionUserObject
  const VariableName & _solution_object_var_name;
};
