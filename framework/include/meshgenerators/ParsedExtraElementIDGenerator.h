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
 * MeshGenerator for adding an extra element integer uses a parsed expression
 */
class ParsedExtraElementIDGenerator : public MeshGenerator, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedExtraElementIDGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// mesh to add the subdomain to
  std::unique_ptr<MeshBase> & _input;

  /// function expression
  const std::string _function;

  /// Names of the extra element ids used in the parsed expression
  const std::vector<ExtraElementIDName> _eeid_names;

  /// extra elem id name
  const std::string & _elem_id_name;

  /// index of the extra elem id
  unsigned int _extra_elem_id;

  /// Indices of the extra element ids used in the parsed expression
  std::vector<unsigned int> _eeid_indices;

  /// function parser object describing the combinatorial geometry
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};
