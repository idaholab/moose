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
 * Modifies the position of one node
 */
class ModifyNodeGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  ModifyNodeGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  Elem * getElemType(const std::string & type);

protected:
  /// Mesh that possibly comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// The node id
  const std::vector<int> _node_id;

  /// The new position of the node
  const std::vector<Point> _new_position;

};

