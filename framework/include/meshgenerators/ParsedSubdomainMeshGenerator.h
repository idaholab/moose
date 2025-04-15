//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParsedSubdomainGeneratorBase.h"

/**
 * MeshGenerator for defining a Subdomain inside or outside of combinatorial geometry
 */
class ParsedSubdomainMeshGenerator : public ParsedSubdomainGeneratorBase
{
public:
  static InputParameters validParams();

  ParsedSubdomainMeshGenerator(const InputParameters & parameters);

protected:
  /// Block ID to assign to the region
  const subdomain_id_type _block_id;

  /**
   * Assign the subdomain id to the element based on the parsed expression
   * @param elem The element to assign the subdomain id to
   */
  void assignElemSubdomainID(Elem * elem) override;

  /**
   * Set block name for the block with new id if applicable
   * @param mesh The mesh to set the block name on
   */
  void setBlockName(std::unique_ptr<MeshBase> & mesh) override;
};
