//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PARSEDSUBDOMAINMESHMODIFIER_H
#define PARSEDSUBDOMAINMESHMODIFIER_H

// MOOSE includes
#include "MeshModifier.h"
#include "FunctionParserUtils.h"

// Forward declerations
class ParsedSubdomainMeshModifier;

template <>
InputParameters validParams<ParsedSubdomainMeshModifier>();

/**
 * MeshModifier for defining a Subdomain inside or outside of combinatorial geometry
 */
class ParsedSubdomainMeshModifier : public MeshModifier, public FunctionParserUtils
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  ParsedSubdomainMeshModifier(const InputParameters & parameters);

  virtual void modify() override;

private:
  /// function expression
  const std::string _function;

  /// Block ID to assign to the region
  const SubdomainID _block_id;

  /// A list of excluded subdomain ids that will not be changed even if they are in the combinatorial geometry
  const std::vector<SubdomainID> _excluded_ids;

  /// function parser object describing the combinatorial geometry
  ADFunctionPtr _func_F;
};

#endif // SUBDOMAINBOUDINGBOX_H
