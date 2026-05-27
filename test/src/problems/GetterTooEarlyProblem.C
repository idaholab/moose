//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov

#include "GetterTooEarlyProblem.h"
#include "Function.h"
#include "UserObject.h"

registerMooseObject("MooseTestApp", GetterTooEarlyProblem);

InputParameters
GetterTooEarlyProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addRequiredParam<MooseEnum>(
      "getter", MooseEnum("function user_object multi_app"), "The getter to call too early");
  params.addParam<FunctionName>("function", "Function to request too early");
  params.addParam<UserObjectName>("user_object", "UserObject to request too early");
  params.addParam<MultiAppName>("multi_app", "MultiApp to request too early");
  return params;
}

GetterTooEarlyProblem::GetterTooEarlyProblem(const InputParameters & params)
  : FEProblem(params), _function(nullptr), _user_object(nullptr)
{
  const auto getter = getParam<MooseEnum>("getter");

  if (getter == "function")
    _function = &getFunction(getParam<FunctionName>("function"));
  else if (getter == "user_object")
    _user_object = &getUserObjectBase(getParam<UserObjectName>("user_object"));
  else if (getter == "multi_app")
    static_cast<void>(getMultiApp(getParam<MultiAppName>("multi_app")));
}
