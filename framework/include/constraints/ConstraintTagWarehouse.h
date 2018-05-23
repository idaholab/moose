//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSTRAINTTAGWAREHOUSE_H
#define CONSTRAINTTAGWAREHOUSE_H

// MOOSE includes
#include "ConstraintWarehouse.h"

/**
 * A storage container for constraints.
 *
 * This function is pretty much a copy of MooseObjectTagWarehouse. But we do not have
 * a good way to get around because ConstraintWarehouse is so special
 */
class ConstraintTagWarehouse : public ConstraintWarehouse
{
public:
  ConstraintTagWarehouse(bool threaded = false);

  /**
   * Update the active status of Kernels
   */
  virtual void updateActive(THREAD_ID tid = 0) override;

  /**
   * Retrieve a moose object warehouse in which every moose object has the given vector tag.
   * If the warehouse is not constructed yet, it will be constructed here and returned. If
   * the warehouse is already cached (it was queried before), we just directly return the
   * cached warehouse.
   */
  ConstraintWarehouse & getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid);

  /**
   * Retrieve a moose object warehouse in which every moose object at least has one of the given
   * vector tags.
   * If the warehouse is not constructed yet, it will be constructed here and returned. If
   * the warehouse is already cached (it was queried before), we just directly return the
   * cached warehouse.
   */
  ConstraintWarehouse & getVectorTagsObjectWarehouse(const std::set<TagID> & tags, THREAD_ID tid);
  /**
   * Retrieve a moose object warehouse in which every moose object has the given matrix tag.
   * If the warehouse is not constructed yet, it will be constructed here and returned. If
   * the warehouse is already cached (it was queried before), we just directly return the
   * cached warehouse.
   */
  ConstraintWarehouse & getMatrixTagObjectWarehouse(TagID tag_id, THREAD_ID tid);

  /**
   * Retrieve a moose object warehouse in which every moose object has one of the given matrix tags.
   * If the warehouse is not constructed yet, it will be constructed here and returned. If
   * the warehouse is already cached (it was queried before), we just directly return the
   * cached warehouse.
   */
  ConstraintWarehouse & getMatrixTagsObjectWarehouse(const std::set<TagID> & tags, THREAD_ID tid);

protected:
  const THREAD_ID _num_threads;

  /// Tag based storage. Map from a tag to a moose object warehouse for vector tags
  std::vector<std::map<TagID, ConstraintWarehouse>> _vector_tag_to_object_warehouse;

  /// std::set<TagID> based storage. Map from a std::set of tags to a moose object warehouse for vector tags.
  std::vector<std::map<std::set<TagID>, ConstraintWarehouse>> _vector_tags_to_object_warehouse;

  /// Tag based storage. Map fro a tag to moose object warehouse for matrix tags.
  std::vector<std::map<TagID, ConstraintWarehouse>> _matrix_tag_to_object_warehouse;

  /// std::set<TagID> based storage. Map from a std::set of tags to moose object warehouse for matrix tags.
  std::vector<std::map<std::set<TagID>, ConstraintWarehouse>> _matrix_tags_to_object_warehouse;
};

#endif // CONSTRAINTTAGWAREHOUSE_H
