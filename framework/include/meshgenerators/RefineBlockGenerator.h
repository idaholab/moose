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
 * MeshGenerator for refining one or more blocks
 */
class RefineBlockGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  
  RefineBlockGenerator(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<MeshBase> generate() override;

private:
  /// Input mesh to modify
  std::unique_ptr<MeshBase> & _input;

  /// List of block(s) to modify
  std::vector<SubdomainName> _block;

  /// The amount of times to refine each block, corresponding to their index in 'block'
  std::vector<int> _refinement;

  /// Toggles whether neighboring level one elements should be refined or not. Defaults to true.
  bool _enable_neighbor_refinement;

  /// The actual function refining the blocks. This is done recursively in order to minimize the number of refinement iterations to as little as possible.
  virtual std::unique_ptr<MeshBase> recursive_refine(std::vector<subdomain_id_type> block_ids,
                                                  std::unique_ptr<MeshBase> & mesh,
                                                  std::vector<int> refinement,
                                                  int max,
                                                  int ref_step = 0);
};
