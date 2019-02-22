#include "ClosureTestAction.h"

template <>
InputParameters
validParams<ClosureTestAction>()
{
  InputParameters params = validParams<TestAction>();

  params.addParam<FunctionName>("T_wall", "Wall temperature function");

  params.set<std::string>("fe_family") = "LAGRANGE";
  params.set<std::string>("fe_order") = "FIRST";

  return params;
}

ClosureTestAction::ClosureTestAction(InputParameters params)
  : TestAction(params),
    _dummy_name("dummy"),
    _T_wall_name("T_wall"),
    _has_T_wall(isParamValid("T_wall")),
    _T_wall_fn(_has_T_wall ? getParam<FunctionName>("T_wall") : "")
{
  _default_use_transient_executioner = true;
}

void
ClosureTestAction::addInitialConditions()
{
  if (_has_T_wall)
    addFunctionIC(_T_wall_name, _T_wall_fn);
}

void
ClosureTestAction::addSolutionVariables()
{
  addSolutionVariable(_dummy_name);
}

void
ClosureTestAction::addNonConstantAuxVariables()
{
  if (_has_T_wall)
    addAuxVariable(_T_wall_name, _fe_family, _fe_order);
}
