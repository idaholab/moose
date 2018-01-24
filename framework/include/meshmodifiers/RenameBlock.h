//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RENAMEBLOCK_H
#define RENAMEBLOCK_H

// MOOSE includes
#include "MeshModifier.h"

// Forward declerations
class RenameBlock;

template <>
InputParameters validParams<RenameBlock>();

/**
 * MeshModifier for re-numbering or re-naming blocks
 */
class RenameBlock : public MeshModifier
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  RenameBlock(const InputParameters & parameters);

private:
  virtual void modify() override;

  std::vector<SubdomainID> _old_block_id;

  std::vector<SubdomainID> _new_block_id;

  std::vector<SubdomainName> _new_block_name;
};

#endif // RENAMEBLOCK_H
