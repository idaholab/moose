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
 * MeshGenerator for constructing Side Sets from Node Sets, and vice versa
 */
class ConvertNodeSetSideSetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  ConvertNodeSetSideSetGenerator(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<MeshBase> generate() override;

private:
  /// Input mesh the opertation will be applied to
  std::unique_ptr<MeshBase> & _input;

  /// Flag signalling if nodes should be constructed from sides
  const bool _convert_side_list_from_node_list;

  /// Flag signalling if sides should be constructed from nodes
  const bool _convert_node_list_from_side_list;

};
