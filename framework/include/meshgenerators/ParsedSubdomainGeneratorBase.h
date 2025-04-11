//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * A base class for mesh generators that Use a parsed expression to assign new subdomain id(s)
 */
class ParsedSubdomainGeneratorBase : public MeshGenerator, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedSubdomainGeneratorBase(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// mesh to add the subdomain to
  std::unique_ptr<MeshBase> & _input;

  /// Block ID to assign to the region
  const subdomain_id_type _block_id;

  /// A list of excluded subdomain ids that will not be changed even if they are in the combinatorial geometry
  std::vector<subdomain_id_type> _excluded_ids;

  /// Names of the extra element ids used in the parsed expression
  const std::vector<ExtraElementIDName> _eeid_names;

  /// Indices of the extra element ids used in the parsed expression
  std::vector<unsigned int> _eeid_indices;

  /// function parser object describing the combinatorial geometry
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);

  /**
   * Initialize the function parser object
   * @param function_expression The expression to parse
   */
  void functionInitialize(const std::string & function_expression);

  virtual void assignElemSubdomainID(Elem * elem) = 0;
};
