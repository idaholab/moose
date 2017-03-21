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

#ifndef PARSEDSUBDOMAINMESHMODIFIER_H
#define PARSEDSUBDOMAINMESHMODIFIER_H

// MOOSE includes
#include "MeshModifier.h"
#include "FunctionParserUtils.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

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
  std::string _function;

  /// Block ID to assign to the region
  SubdomainID _block_id;

  /// A list of excluded subdomain ids that will not be changed even if they are in the combinatorial geometry
  std::vector<SubdomainID> _excluded_ids;

  /// function parser object describing the combinatorial geometry
  ADFunctionPtr _func_F;
};

#endif // SUBDOMAINBOUDINGBOX_H
