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

  std::vector<SubdomainName> _old_block_name;

  std::vector<SubdomainID> _new_block_id;

  std::vector<SubdomainName> _new_block_name;

  /**
   * Given a new_block_id, provide a block name, based
   * on the old block names provided (or deduced from
   * the old_block_ids provided).
   *
   * Eg, if
   * old_block_name = 'block1 block2'
   * new_block_id   = '4      5'
   * Then
   * newBlockName(4) = block1
   * newBlockName(5) = block2
   *
   * In the case of merging blocks, the *first encountered*
   * block's name is used.
   * Eg, if
   * old_block_name = 'asdf block4 block1'
   * new_block_id   = '3    1      1'
   * then
   * newBlockName(1) = block4, because we look along
   * new_block_id until we first encounter 1, and then get the corresponding name
   * @param new_block_id the new block's ID number
   * @return the name that will be given to that block
   */
  const SubdomainName newBlockName(const SubdomainID & new_block_id);

  /**
   * Given a new_block_name, provide a block ID, based
   * on the old block IDs provided (or deduced from
   * the old_block_names provided).
   *
   * Eg, if
   * old_block_id =   '4      5'
   * new_block_name = 'block1 block2'
   * Then
   * newBlockID(block1) = 4
   * newBlockID(block2) = 5
   *
   * In the case of merging blocks, the *first encountered*
   * block's ID is used.
   * Eg, if
   * old_block_id =   '3    1      1'
   * new_block_name = 'asdf block4 block4'
   * then
   * newBlockID(block4) = 1, because we look along
   * new_block_name until we first encounter block4, and then get the corresponding ID
   * @param new_block_name the new block's name
   * @return the ID number that will be given to that block
   */
  const SubdomainID newBlockID(const SubdomainName & new_block_name);
};

#endif // RENAMEBLOCK_H
