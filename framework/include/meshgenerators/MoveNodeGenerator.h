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
 * Modifies the position of one or more node(s)
 */
class MoveNodeGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  MoveNodeGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that possibly comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// The id(s) of the node(s) to be moved
  const std::vector<dof_id_type> _node_id;

  /// The new position(s) of the node
  const std::vector<Point> * _new_position;

  /// The shift(s) to apply to each node
  const std::vector<Point> * _shift_position;
};
