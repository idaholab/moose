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
 * MeshGenerator for defining a Sideset by a parsed expression and
 * optionally by looking at the subdomain a side's element belongs to
 * and the side's normal vector
 */
class ParsedGenerateSideset : public SideSetsGeneratorBase, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedGenerateSideset(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// mesh to add the sidesets to
  std::unique_ptr<MeshBase> & _input;

  /// function expression
  std::string _function;

  /// name of the new sideset
  BoundaryName _sideset_name;

  /// whether to check boundary ids when adding sides or not
  bool _check_boundaries;

  /// whether to check subdomain ids when adding sides or not
  bool _check_subdomains;

  /// whether to check neighbor subdomain ids when adding sides or not
  bool _check_neighbor_subdomains;

  /// whether to check normals when adding sides or not
  bool _check_normal;

  /// A list of included subdomain ids that the side has to be part of
  std::vector<subdomain_id_type> _included_ids;

  /// A list of included neighbor subdomain ids
  std::vector<subdomain_id_type> _included_neighbor_ids;

  /// A normal vector that (if provided) is compared against side's normals
  Point _normal;

  /// function parser object describing the combinatorial geometry
  SymFunctionPtr _func_F;

  usingFunctionParserUtilsMembers(false);
};
