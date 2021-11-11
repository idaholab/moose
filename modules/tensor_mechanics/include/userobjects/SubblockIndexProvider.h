//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "libmesh/elem.h"

using libMesh::Elem;

/**
 * Abstract base class for user objects that provide an index for a given element that is
 * independent of the block id, so that behavior can be different on subsets of element blocks.
 * This is used to apply independent generalized plane constraints to subsets of element blocks.
 */
class SubblockIndexProvider
{
public:
  virtual ~SubblockIndexProvider(){};
  /**
   * The index of subblock this element is on.
   */
  virtual unsigned int getSubblockIndex(const Elem & /* elem */) const = 0;
  /**
   * The max index of subblock.
   */
  virtual unsigned int getMaxSubblockIndex() const = 0;
};
