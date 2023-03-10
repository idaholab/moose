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
 * Generates individual elements given a list of nodal positions
 */
class GenericFaceGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  GenericFaceGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that possibly comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// The nodal positions
  const std::vector<Point> & _nodal_positions;

  /// The connectivity of the elements to the nodes
  const std::vector<std::vector<dof_id_type>> & _element_connectivity;
};
