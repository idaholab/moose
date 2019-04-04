#include "SolutionInitialCondition.h"
#include "SolutionUserObject.h"
#include "MooseMesh.h"

registerMooseObject("THMApp", SolutionInitialCondition);

template <>
InputParameters
validParams<SolutionInitialCondition>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("solution_uo",
                                          "The SolutionUserObject to extract data from.");
  params.addRequiredParam<VariableName>(
      "from_variable", "The name of the variable in the file that is to be extracted");
  return params;
}

SolutionInitialCondition::SolutionInitialCondition(const InputParameters & parameters)
  : InitialCondition(parameters),
    _solution_object(getUserObject<SolutionUserObject>("solution_uo")),
    _solution_object_var_name(getParam<VariableName>("from_variable"))
{
}

Real
SolutionInitialCondition::value(const Point & p)
{
  return _solution_object.pointValue(0., p, _solution_object_var_name);
}
