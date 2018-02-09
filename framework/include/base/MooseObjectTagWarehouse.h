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

  bool hasVectorTaggedObjectWarehouse() { return _vector_tag_to_object_warehourse.size() > 0; }

  bool hasMatrixTaggedObjectWarehouse() { return _matrixTagID_to_object_warehourse.size() > 0; }

protected:
  /// Tag based storage. Tag to object warehouse
  std::map<TagID, MooseObjectTagWarehouse<T>> _vector_tag_to_object_warehourse;

  /// Object warehouse associated with a few tags
  std::map<std::vector<TagID>, MooseObjectTagWarehouse<T>> _vector_tags_to_object_warehourse;

  /// Tag to Matrix Object
  std::map<TagID, MooseObjectTagWarehouse<T>> _matrixTagID_to_object_warehourse;
};

template <typename T>
MooseObjectTagWarehouse<T>::MooseObjectTagWarehouse(bool threaded /*=true*/)
  : MooseObjectWarehouse<T>(threaded)
{
}

template <typename T>
MooseObjectTagWarehouse<T> &
MooseObjectTagWarehouse<T>::getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  const auto & house_end = _vector_tag_to_object_warehourse.end();
  const auto & tag_warehouse = _vector_tag_to_object_warehourse.find(tag_id);

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
          _vector_tag_to_object_warehourse[tag_id].addObject(object, tid);

          break;
        }
      }
    }
    return _vector_tag_to_object_warehourse[tag_id];
  }
}

template <typename T>
MooseObjectTagWarehouse<T> &
MooseObjectTagWarehouse<T>::getVectorTagsObjectWarehouse(std::vector<TagID> & v_tags, THREAD_ID tid)
{
  const auto & house_end = _vector_tags_to_object_warehourse.end();
  const auto & tags_warehouse = _vector_tags_to_object_warehourse.find(v_tags);

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
          _vector_tags_to_object_warehourse[v_tags].addObject(object, tid);

          // Then we should work for next object
          break;
        }
      }
    }
    return _vector_tags_to_object_warehourse[v_tags];
  }
}

#endif // MOOSEOBJECTTAGWAREHOUSE_H
