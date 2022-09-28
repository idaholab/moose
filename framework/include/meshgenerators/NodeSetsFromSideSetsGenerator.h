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
 * MeshGenerator for constructing node sets from side sets
 */
class NodeSetsFromSideSetsGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  NodeSetsFromSideSetsGenerator(const InputParameters & parameters);

protected:
  std::unique_ptr<MeshBase> generate() override;

private:
  /// Input mesh the operation will be applied to
  std::unique_ptr<MeshBase> & _input;
};
