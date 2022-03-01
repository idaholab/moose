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

/**
 * MeshGenerator for assigning a subdomain ID to all elements
 */
class SubdomainIDGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SubdomainIDGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the input mesh, which may be output by another mesh generator
  std::unique_ptr<MeshBase> & _input;

  /// The subdomain ID to assign to every elemennt
  SubdomainID _subdomain_id;
};
