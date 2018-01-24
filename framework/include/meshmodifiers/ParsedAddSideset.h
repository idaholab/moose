//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PARSEDADDSIDESET_H
#define PARSEDADDSIDESET_H

// MOOSE includes
#include "AddSideSetsBase.h"
#include "FunctionParserUtils.h"

// Forward declerations
class ParsedAddSideset;

template <>
InputParameters validParams<ParsedAddSideset>();

/**
 * MeshModifier for defining a Sideset by a parsed expression and
 * optionally by looking at the subdomain a side's element belongs to
 * and the side's normal vector
 */
class ParsedAddSideset : public AddSideSetsBase, public FunctionParserUtils
{
public:
  ParsedAddSideset(const InputParameters & parameters);
  virtual void modify() override;

private:
  /// function expression
  std::string _function;

  /// name of the new sideset
  BoundaryName _sideset_name;

  /// whether to check subdomain ids when adding sides or not
  bool _check_subdomains;

  /// whether to check normals when adding sides or not
  bool _check_normal;

  /// A list of included subdomain ids that the side has to be part of
  std::vector<SubdomainID> _included_ids;

  /// A normal vector that (if provided) is compared against side's normals
  Point _normal;

  /// function parser object describing the combinatorial geometry
  ADFunctionPtr _func_F;
};

#endif // PARSEDADDSIDESET_H
