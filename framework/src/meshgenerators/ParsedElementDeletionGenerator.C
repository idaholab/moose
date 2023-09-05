//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedElementDeletionGenerator.h"

registerMooseObject("MooseApp", ParsedElementDeletionGenerator);

InputParameters
ParsedElementDeletionGenerator::validParams()
{
  InputParameters params = ElementDeletionGeneratorBase::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addClassDescription(
      "Removes elements such that the parsed expression is evaluated as strictly positive. "
      "The parameters of the parsed expression can be the X,Y,Z coordinates of the "
      "element vertex average (must be 'x','y','z' in the expression), the element volume "
      "(must be 'volume' in the expression) and the element id ('id' in the expression).");
  params.addRequiredParam<ParsedFunctionExpression>(
      "expression", "Expression to evaluate to decide whether an element should be deleted");
  params.addParam<std::vector<std::string>>("constant_names",
                                            "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  return params;
}

ParsedElementDeletionGenerator::ParsedElementDeletionGenerator(const InputParameters & parameters)
  : ElementDeletionGeneratorBase(parameters), FunctionParserUtils<false>(parameters)
{
  _function = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_function);

  // add the constant expressions
  addFParserConstants(_function,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse function
  if (_function->Parse(getParam<ParsedFunctionExpression>("expression"), "x,y,z,volume,id") >= 0)
    paramError("expression", "Invalid function: ", _function->ErrorMsg());

  _func_params.resize(5);
}

bool
ParsedElementDeletionGenerator::shouldDelete(const Elem * elem)
{
  const auto vertex_average = elem->vertex_average();
  for (const auto i : make_range(3))
    _func_params[i] = vertex_average(i);
  _func_params[3] = elem->volume();
  _func_params[4] = elem->id();

  return evaluate(_function, "expression") > 0;
}
