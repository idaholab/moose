//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "FunctionParserUtils.h"

#include <array>

/*
 * A mesh generator that applies an arbitrary transformation to the nodal coordinates of a mesh
 */
class ParsedNodeTransformGenerator : public MeshGenerator, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedNodeTransformGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// names of each component function parameter
  static const std::string _func_name[];

  /// the input mesh
  std::unique_ptr<MeshBase> & _input;

  /// the node position functions
  std::array<SymFunctionPtr, 3> _functions;
};
