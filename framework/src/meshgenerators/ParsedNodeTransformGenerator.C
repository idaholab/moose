//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedNodeTransformGenerator.h"
#include "CastUniquePointer.h"

#include <libmesh/int_range.h>

registerMooseObject("MooseApp", ParsedNodeTransformGenerator);

const std::string ParsedNodeTransformGenerator::_func_name[] = {
    "x_function", "y_function", "z_function"};

InputParameters
ParsedNodeTransformGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Applies a transform to a the x,y,z coordinates of a Mesh");
  params.addParam<ParsedFunctionExpression>(
      _func_name[0], "x", "Function for the updated x component of the node");
  params.addParam<ParsedFunctionExpression>(
      _func_name[1], "y", "Function for the updated y component of the node");
  params.addParam<ParsedFunctionExpression>(
      _func_name[2], "z", "Function for the updated z component of the node");

  params.addParam<std::vector<std::string>>(
      "constant_names", {}, "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  return params;
}

ParsedNodeTransformGenerator::ParsedNodeTransformGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), FunctionParserUtils<false>(parameters), _input(getMesh("input"))
{
  for (const auto i : index_range(_functions))
  {
    // Create parsed function
    _functions[i] = std::make_shared<SymFunction>();
    parsedFunctionSetup(_functions[i],
                        getParam<ParsedFunctionExpression>(_func_name[i]),
                        "x,y,z",
                        getParam<std::vector<std::string>>("constant_names"),
                        getParam<std::vector<std::string>>("constant_expressions"),
                        comm());
  }

  _func_params.resize(3);
}

std::unique_ptr<MeshBase>
ParsedNodeTransformGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  for (auto & node : mesh->node_ptr_range())
  {
    for (const auto i : make_range(3))
      _func_params[i] = (*node)(i);

    for (const auto i : make_range(3))
      (*node)(i) = evaluate(_functions[i], _func_name[i]);
  }

  return mesh;
}
