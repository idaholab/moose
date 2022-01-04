#include "ScalarSolutionInitialCondition.h"
#include "SolutionUserObject.h"
#include "MooseMesh.h"

registerMooseObject("THMApp", ScalarSolutionInitialCondition);

InputParameters
ScalarSolutionInitialCondition::validParams()
{
  InputParameters params = ScalarInitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("solution_uo",
                                          "The SolutionUserObject to extract data from.");
  params.addRequiredParam<VariableName>(
      "from_variable", "The name of the variable in the file that is to be extracted");
  return params;
}

ScalarSolutionInitialCondition::ScalarSolutionInitialCondition(const InputParameters & parameters)
  : ScalarInitialCondition(parameters),
    _solution_object(getUserObject<SolutionUserObject>("solution_uo")),
    _solution_object_var_name(getParam<VariableName>("from_variable"))
{
}

Real
ScalarSolutionInitialCondition::value()
{
  return _solution_object.scalarValue(0., _solution_object_var_name);
}
