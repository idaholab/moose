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

  /**
   * Update the active status of Kernels
   */
  virtual void updateActive(THREAD_ID tid = 0) override;

  MooseObjectWarehouse<T> & getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid);

  MooseObjectWarehouse<T> & getVectorTagsObjectWarehouse(std::set<TagID> & tags, THREAD_ID tid);

  MooseObjectWarehouse<T> & getMatrixTagObjectWarehouse(TagID tag_id, THREAD_ID tid);

  MooseObjectWarehouse<T> & getMatrixTagsObjectWarehouse(std::set<TagID> & tags, THREAD_ID tid);

protected:
  /// Tag based storage. Tag to object warehouse
  std::map<TagID, MooseObjectWarehouse<T>> _vector_tag_to_object_warehouse;

  /// Object warehouse associated with a few tags
  std::map<std::set<TagID>, MooseObjectWarehouse<T>> _vector_tags_to_object_warehouse;

  /// Tag to Matrix Object
  std::map<TagID, MooseObjectWarehouse<T>> _matrix_tag_to_object_warehouse;

  /// std::vector<TagID> based storage
  std::map<std::set<TagID>, MooseObjectWarehouse<T>> _matrix_tags_to_object_warehouse;

private:
  /// a static mutex used in warehouse
  static Threads::spin_mutex _moose_object_tag_warehouse_mutex;
};

template <typename T>
Threads::spin_mutex MooseObjectTagWarehouse<T>::_moose_object_tag_warehouse_mutex;

template <typename T>
MooseObjectTagWarehouse<T>::MooseObjectTagWarehouse(bool threaded /*=true*/)
  : MooseObjectWarehouse<T>(threaded)
{
}

template <typename T>
void
MooseObjectTagWarehouse<T>::updateActive(THREAD_ID tid)
{
  MooseObjectWarehouse<T>::updateActive(tid);

  for (auto & it : _vector_tag_to_object_warehouse)
    it.second.updateActive(tid);

  for (auto & it : _vector_tags_to_object_warehouse)
    it.second.updateActive(tid);

  for (auto & it : _matrix_tag_to_object_warehouse)
    it.second.updateActive(tid);

  for (auto & it : _matrix_tags_to_object_warehouse)
    it.second.updateActive(tid);
}

template <typename T>
MooseObjectWarehouse<T> &
MooseObjectTagWarehouse<T>::getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  const auto & house_end = _vector_tag_to_object_warehouse.end();
  const auto & tag_warehouse = _vector_tag_to_object_warehouse.find(tag_id);

  if (tag_warehouse != house_end)
    return tag_warehouse->second;
  else
  {
    // std::map is not thread-safe for writes
    Threads::spin_mutex::scoped_lock lock(_moose_object_tag_warehouse_mutex);
    _vector_tag_to_object_warehouse[tag_id];
  }

  // Now add actual moose objects into warehouse
  const auto & objects = MooseObjectWarehouse<T>::getActiveObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getVectorTags();
    for (auto & tag : tags)
    {
      if (tag == tag_id)
      {
        // Tag based storage
        _vector_tag_to_object_warehouse[tag_id].addObject(object, tid);

        break;
      }
    }
  }

  return _vector_tag_to_object_warehouse[tag_id];
}

template <typename T>
MooseObjectWarehouse<T> &
MooseObjectTagWarehouse<T>::getMatrixTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  const auto & house_end = _matrix_tag_to_object_warehouse.end();
  const auto & tag_warehouse = _matrix_tag_to_object_warehouse.find(tag_id);

  if (tag_warehouse != house_end)
    return tag_warehouse->second;
  else
  {
    // std::map is not thread-safe for writes
    {
      Threads::spin_mutex::scoped_lock lock(_moose_object_tag_warehouse_mutex);
      _matrix_tag_to_object_warehouse[tag_id];
    }
  }

  // Add moose objects to matrix-tag warehouse
  const auto & objects = MooseObjectWarehouse<T>::getActiveObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getMatrixTags();
    for (auto & tag : tags)
    {
      if (tag == tag_id)
      {
        // Tag based storage
        _matrix_tag_to_object_warehouse[tag_id].addObject(object, tid);

        break;
      }
    }
  }

  return _matrix_tag_to_object_warehouse[tag_id];
}

template <typename T>
MooseObjectWarehouse<T> &
MooseObjectTagWarehouse<T>::getVectorTagsObjectWarehouse(std::set<TagID> & v_tags, THREAD_ID tid)
{
  const auto & house_end = _vector_tags_to_object_warehouse.end();
  const auto & tags_warehouse = _vector_tags_to_object_warehouse.find(v_tags);

  if (tags_warehouse != house_end)
    return tags_warehouse->second;
  else
  {
    // std::map is not thread-safe for writes
    Threads::spin_mutex::scoped_lock lock(_moose_object_tag_warehouse_mutex);
    _vector_tags_to_object_warehouse[v_tags];
  }

  // Add moose objects to vector-tags warehouse
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
        _vector_tags_to_object_warehouse[v_tags].addObject(object, tid);
        // Then we should work for next object
        break;
      }
    }
  }

  return _vector_tags_to_object_warehouse[v_tags];
}

template <typename T>
MooseObjectWarehouse<T> &
MooseObjectTagWarehouse<T>::getMatrixTagsObjectWarehouse(std::set<TagID> & m_tags, THREAD_ID tid)
{
  const auto & house_end = _matrix_tags_to_object_warehouse.end();
  const auto & tags_warehouse = _matrix_tags_to_object_warehouse.find(m_tags);

  if (tags_warehouse != house_end)
    return tags_warehouse->second;
  else
  {
    // std::map is not thread-safe for writes
    Threads::spin_mutex::scoped_lock lock(_moose_object_tag_warehouse_mutex);
    _matrix_tags_to_object_warehouse[m_tags];
  }

  const auto & objects = MooseObjectWarehouse<T>::getActiveObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getMatrixTags();
    const auto & tags_end = tags.end();
    for (auto & m_tag : m_tags)
    {
      const auto & tag_found = tags.find(m_tag);
      // Object contains at least one of required tags
      if (tag_found != tags_end)
      {
        // std::vector<Tag> based storage
        _matrix_tags_to_object_warehouse[m_tags].addObject(object, tid);
        // Then we should work for next object
        break;
      }
    }
  }

  return _matrix_tags_to_object_warehouse[m_tags];
}

#endif // MOOSEOBJECTTAGWAREHOUSE_H
