//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementDeletionGeneratorBase.h"

/**
 * MeshGenerator for removing blocks from the mesh
 */
class BlockDeletionGenerator : public ElementDeletionGeneratorBase
{
public:
  static InputParameters validParams();

  BlockDeletionGenerator(const InputParameters & parameters);

protected:
  virtual std::unique_ptr<MeshBase> generate() override;
  virtual bool shouldDelete(const Elem * elem) override;

private:
  /// Ids of the blocks to be removed
  std::vector<SubdomainID> _block_ids;
};
