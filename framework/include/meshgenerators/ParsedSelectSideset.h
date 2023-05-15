//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideSetsGeneratorBase.h"
#include "FunctionParserUtils.h"
#include "libmesh/point.h"

/**
 * Defines a new sideset, which is the intersect of a existing sideset and
* the combinatorial geometry.
 */

class ParsedSelectSideset : public SideSetsGeneratorBase, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedSelectSideset(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// mesh to add the sidesets to
  std::unique_ptr<MeshBase> & _input;

  /// function expression
  std::string _function;

  /// name of the new boundary
  BoundaryName _new_boundary_name;

  /// name of the old boundary
  BoundaryName _old_boundary_name;

  /// function parser object describing the combinatorial geometry
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};

