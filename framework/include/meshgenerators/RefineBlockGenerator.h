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
 * MeshGenerator for removing blocks from the mesh
 */
class RefineBlockGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  virtual std::unique_ptr<MeshBase> recurs_refine(std::vector<subdomain_id_type> block_ids, std::unique_ptr<MeshBase> &mesh, std::vector<int> _refinement, int max, int ref_step = 0);

  RefineBlockGenerator(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<MeshBase> generate() override;

private:
  std::unique_ptr<MeshBase> & _input;
  std::vector<SubdomainName> _block;
  std::vector<int> _refinement;
  bool _enable_neighbors;
  
};
