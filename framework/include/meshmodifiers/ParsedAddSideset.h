/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PARSEDADDSIDESET_H
#define PARSEDADDSIDESET_H

// MOOSE includes
#include "AddSideSetsBase.h"
#include "FunctionParserUtils.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

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
