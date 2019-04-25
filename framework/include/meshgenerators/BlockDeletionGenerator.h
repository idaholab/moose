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

// Forward declarations
class BlockDeletionGenerator;

template <>
InputParameters validParams<BlockDeletionGenerator>();

/**
 * MeshGenerator for
 */
class BlockDeletionGenerator : public ElementDeletionGeneratorBase
{
public:
  BlockDeletionGenerator(const InputParameters & parameters);

protected:
  virtual bool shouldDelete(const Elem * elem) override;

private:
  ///Defines the block to be removed
  const SubdomainID _block_id;
};

