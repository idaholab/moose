#pragma once

#include "InitialCondition.h"

class SolutionUserObject;

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

public:
  static InputParameters validParams();
};
