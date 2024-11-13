//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedChainControl.h"
#include "ChainControlParsedFunctionWrapper.h"

registerMooseObject("MooseApp", ParsedChainControl);

InputParameters
ParsedChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();
  params += MooseParsedFunctionBase::validParams();

  params.addClassDescription(
      "Parses and evaluates a function expression to populate a control value.");

  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "Expression to parse to create the function to evaluate");
  params.addParam<Point>("point", Point(), "Spatial point at which to evaluate the function");

  return params;
}

ParsedChainControl::ParsedChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    MooseParsedFunctionBase(parameters),
    _function_expression(verifyFunction(getParam<std::string>("expression"))),
    _value(declareChainControlData<Real>("value")),
    _point(getParam<Point>("point"))
{
}

void
ParsedChainControl::buildFunction()
{
  if (!_function_ptr)
  {
    THREAD_ID tid = 0;
    if (isParamValid("_tid"))
      tid = getParam<THREAD_ID>("_tid");

    _function_ptr = std::make_unique<ChainControlParsedFunctionWrapper>(
        getMooseApp(), _pfb_feproblem, _function_expression, _vars, _vals, tid);
  }
}

void
ParsedChainControl::init()
{
  buildFunction();

  // Add dependencies for the chain control data used in the function expression
  for (auto & data_ptr : _function_ptr->getRealChainControlData())
    addChainControlDataDependency(data_ptr->name());
  for (auto & data_ptr : _function_ptr->getBoolChainControlData())
    addChainControlDataDependency(data_ptr->name());
}

void
ParsedChainControl::execute()
{
  _value = _function_ptr->evaluate(_t, _point);
}
