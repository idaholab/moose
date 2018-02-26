//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEOBJECTTAGWAREHOUSE_H
#define MOOSEOBJECTTAGWAREHOUSE_H

// MOOSE includes
#include "MooseObjectWarehouse.h"

/**
 * A storage container for MooseObjects that inherit from SetupInterface.
 *
 * Objects that inherit from SetupInterface have various functions (e.g., initialSetup). This
 * class provides convenience functions for looping over all active Objects stored in the warehouse
 * and calling the setup methods.
 */
template <typename T>
class MooseObjectTagWarehouse : public MooseObjectWarehouse<T>
{
public:
  /**
   * Constructor.
   * @param threaded When true (default) threaded storage is enabled.
   */
  MooseObjectTagWarehouse(bool threaded = true);

  ///@{

  MooseObjectTagWarehouse<T> & getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid);

  MooseObjectTagWarehouse<T> & getVectorTagsObjectWarehouse(std::vector<TagID> & tags,
                                                            THREAD_ID tid);

  MooseObjectTagWarehouse<T> & getMatrixTagObjectWarehouse(TagID tag_id, THREAD_ID tid);

  MooseObjectTagWarehouse<T> & getMatrixTagsObjectWarehouse(std::vector<TagID> & tags,
                                                            THREAD_ID tid);
protected:
  const THREAD_ID _num_threads;

  /// Tag based storage. Tag to object warehouse
  std::vector<std::map<TagID, MooseObjectTagWarehouse<T>>> _vector_tag_to_object_warehouse;

  /// Object warehouse associated with a few tags
  std::vector<std::map<std::vector<TagID>, MooseObjectTagWarehouse<T>>>
      _vector_tags_to_object_warehouse;

  /// Tag to Matrix Object
  std::vector<std::map<TagID, MooseObjectTagWarehouse<T>>> _matrix_tag_to_object_warehouse;

  /// std::vector<TagID> based storage
  std::vector<std::map<std::vector<TagID>, MooseObjectTagWarehouse<T>>>
      _matrix_tags_to_object_warehouse;
};

template <typename T>
MooseObjectTagWarehouse<T>::MooseObjectTagWarehouse(bool threaded /*=true*/)
  : MooseObjectWarehouse<T>(threaded),
    _num_threads(threaded ? libMesh::n_threads() : 1),
    _vector_tag_to_object_warehouse(_num_threads),
    _vector_tags_to_object_warehouse(_num_threads),
    _matrix_tag_to_object_warehouse(_num_threads),
    _matrix_tags_to_object_warehouse(_num_threads)
{
}

template <typename T>
MooseObjectTagWarehouse<T> &
MooseObjectTagWarehouse<T>::getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  auto & vector_tag_to_object_warehourse = _vector_tag_to_object_warehouse[tid];

  const auto & house_end = vector_tag_to_object_warehourse.end();
  const auto & tag_warehouse = vector_tag_to_object_warehourse.find(tag_id);

  if (tag_warehouse != house_end)
    return tag_warehouse->second;
  else
  {
    const auto & objects = MooseObjectWarehouse<T>::getActiveObjects(tid);
    for (auto & object : objects)
    {
      auto & tags = object->getVectorTags();
      for (auto & tag : tags)
      {
        if (tag == tag_id)
        {
          // Tag based storage
          vector_tag_to_object_warehourse[tag_id].addObject(object, tid);

          break;
        }
      }
    }
    return vector_tag_to_object_warehourse[tag_id];
  }
}

template <typename T>
MooseObjectTagWarehouse<T> &
MooseObjectTagWarehouse<T>::getMatrixTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  auto & matrix_tag_to_object_warehourse = _matrix_tag_to_object_warehouse[tid];

  const auto & house_end = matrix_tag_to_object_warehourse.end();
  const auto & tag_warehouse = matrix_tag_to_object_warehourse.find(tag_id);

  if (tag_warehouse != house_end)
    return tag_warehouse->second;
  else
  {
    const auto & objects = MooseObjectWarehouse<T>::getActiveObjects(tid);
    for (auto & object : objects)
    {
      auto & tags = object->getMatrixTags();
      for (auto & tag : tags)
      {
        if (tag == tag_id)
        {
          // Tag based storage
          matrix_tag_to_object_warehourse[tag_id].addObject(object, tid);

          break;
        }
      }
    }
    return matrix_tag_to_object_warehourse[tag_id];
  }
}

template <typename T>
MooseObjectTagWarehouse<T> &
MooseObjectTagWarehouse<T>::getVectorTagsObjectWarehouse(std::vector<TagID> & v_tags, THREAD_ID tid)
{
  auto & vector_tags_to_object_warehourse = _vector_tags_to_object_warehouse[tid];

  const auto & house_end = vector_tags_to_object_warehourse.end();
  const auto & tags_warehouse = vector_tags_to_object_warehourse.find(v_tags);

  if (tags_warehouse != house_end)
    return tags_warehouse->second;
  else
  {
    const auto & objects = MooseObjectWarehouse<T>::getActiveObjects(tid);
    for (auto & object : objects)
    {
      auto & tags = object->getVectorTags();
      const auto & tags_end = tags.end();
      for (auto & v_tag : v_tags)
      {
        const auto & tag_found = tags.find(v_tag);
        // Object contains at least one of required tags
        if (tag_found != tags_end)
        {
          // std::vector<Tag> based storage
          vector_tags_to_object_warehourse[v_tags].addObject(object, tid);

          // Then we should work for next object
          break;
        }
      }
    }
    return vector_tags_to_object_warehourse[v_tags];
  }
}

template <typename T>
MooseObjectTagWarehouse<T> &
MooseObjectTagWarehouse<T>::getMatrixTagsObjectWarehouse(std::vector<TagID> & v_tags, THREAD_ID tid)
{
  auto & matrix_tags_to_object_warehourse = _matrix_tags_to_object_warehouse[tid];

  const auto & house_end = matrix_tags_to_object_warehourse.end();
  const auto & tags_warehouse = matrix_tags_to_object_warehourse.find(v_tags);

  if (tags_warehouse != house_end)
    return tags_warehouse->second;
  else
  {
    const auto & objects = MooseObjectWarehouse<T>::getActiveObjects(tid);
    for (auto & object : objects)
    {
      auto & tags = object->getMatrixTags();
      const auto & tags_end = tags.end();
      for (auto & v_tag : v_tags)
      {
        const auto & tag_found = tags.find(v_tag);
        // Object contains at least one of required tags
        if (tag_found != tags_end)
        {
          // std::vector<Tag> based storage
          matrix_tags_to_object_warehourse[v_tags].addObject(object, tid);

          // Then we should work for next object
          break;
        }
      }
    }
    return matrix_tags_to_object_warehourse[v_tags];
  }
}

#endif // MOOSEOBJECTTAGWAREHOUSE_H
