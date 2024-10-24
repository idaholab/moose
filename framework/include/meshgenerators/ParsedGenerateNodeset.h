//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionParserUtils.h"
#include "NodeSetsGeneratorBase.h"
#include "libmesh/point.h"

/**
 * MeshGenerator for defining a nodeset by a parsed expression
 */
class ParsedGenerateNodeset : public NodeSetsGeneratorBase, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedGenerateNodeset(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// function expression
  std::string _function;

  /// function parser object describing the combinatorial geometry
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};
