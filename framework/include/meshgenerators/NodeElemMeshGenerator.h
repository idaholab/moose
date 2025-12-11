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

/**
 * Generates a NodeElem at a given point.
 */
class NodeElemMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  NodeElemMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Point where NodeElem is to be located
  const Point & _point;
  /// Subdomain ID to assign to the new element
  const dof_id_type _subdomain_id;
  /// Subdomain name to assign to the new element
  const SubdomainName & _subdomain_name;
};
