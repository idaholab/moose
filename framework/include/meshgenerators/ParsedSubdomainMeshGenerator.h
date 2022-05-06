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

/**
 * MeshGenerator for defining a Subdomain inside or outside of combinatorial geometry
 */
class ParsedSubdomainMeshGenerator : public MeshGenerator, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedSubdomainMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// mesh to add the subdomain to
  std::unique_ptr<MeshBase> & _input;

  /// function expression
  const std::string _function;

  /// Block ID to assign to the region
  const subdomain_id_type _block_id;

  /// A list of excluded subdomain ids that will not be changed even if they are in the combinatorial geometry
  std::vector<subdomain_id_type> _excluded_ids;

  /// function parser object describing the combinatorial geometry
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};
