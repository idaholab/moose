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
 * MeshGenerator for transcribing Node Sets into Side Sets, and/or Side Sets into Node Sets
 */
class ConvertNodeSetSideSetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  ConvertNodeSetSideSetGenerator(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<MeshBase> generate() override;

private:
  /// Input mesh to refine
  std::unique_ptr<MeshBase> & _input;

  /// Flag signalling if nodes should be transcribed to sides
  const bool _convert_side_list_from_node_list;

  /// Flag signalling if sides should be transcribed to nodes
  const bool _convert_node_list_from_side_listc;

};
