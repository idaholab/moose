//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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

  /**
   * Retrieve a moose object warehouse in which every moose object has the given vector tag.
   * If the warehouse is not constructed yet, it will be constructed here and returned. If
   * the warehouse is already cached (it was queried before), we just directly return the
   * cached warehouse.
   */
  MooseObjectWarehouse<T> & getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid);

  /**
   * Retrieve a moose object warehouse in which every moose object at least has one of the given
   * vector tags.
   * If the warehouse is not constructed yet, it will be constructed here and returned. If
   * the warehouse is already cached (it was queried before), we just directly return the
   * cached warehouse.
   */
  MooseObjectWarehouse<T> & getVectorTagsObjectWarehouse(const std::set<TagID> & tags,
                                                         THREAD_ID tid);
  /**
   * Retrieve a moose object warehouse in which every moose object has the given matrix tag.
   * If the warehouse is not constructed yet, it will be constructed here and returned. If
   * the warehouse is already cached (it was queried before), we just directly return the
   * cached warehouse.
   */
  MooseObjectWarehouse<T> & getMatrixTagObjectWarehouse(TagID tag_id, THREAD_ID tid);

  /**
   * Retrieve a moose object warehouse in which every moose object has one of the given matrix tags.
   * If the warehouse is not constructed yet, it will be constructed here and returned. If
   * the warehouse is already cached (it was queried before), we just directly return the
   * cached warehouse.
   */
  MooseObjectWarehouse<T> & getMatrixTagsObjectWarehouse(const std::set<TagID> & tags,
                                                         THREAD_ID tid);

protected:
  const THREAD_ID _num_threads;

  /// Tag based storage. Map from a tag to a moose object warehouse for vector tags
  std::vector<std::map<TagID, MooseObjectWarehouse<T>>> _vector_tag_to_object_warehouse;

  /// std::set<TagID> based storage. Map from a std::set of tags to a moose object warehouse for vector tags.
  std::vector<std::map<std::set<TagID>, MooseObjectWarehouse<T>>> _vector_tags_to_object_warehouse;

  /// Tag based storage. Map fro a tag to moose object warehouse for matrix tags.
  std::vector<std::map<TagID, MooseObjectWarehouse<T>>> _matrix_tag_to_object_warehouse;

  /// std::set<TagID> based storage. Map from a std::set of tags to moose object warehouse for matrix tags.
  std::vector<std::map<std::set<TagID>, MooseObjectWarehouse<T>>> _matrix_tags_to_object_warehouse;
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
void
MooseObjectTagWarehouse<T>::updateActive(THREAD_ID tid)
{
  MooseObjectWarehouse<T>::updateActive(tid);

  for (auto & it : _vector_tag_to_object_warehouse[tid])
    it.second.updateActive(tid);

  for (auto & it : _vector_tags_to_object_warehouse[tid])
    it.second.updateActive(tid);

  for (auto & it : _matrix_tag_to_object_warehouse[tid])
    it.second.updateActive(tid);

  for (auto & it : _matrix_tags_to_object_warehouse[tid])
    it.second.updateActive(tid);
}

template <typename T>
MooseObjectWarehouse<T> &
MooseObjectTagWarehouse<T>::getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  auto & vector_tag_to_object_warehouse = _vector_tag_to_object_warehouse[tid];

  const auto & house_end = vector_tag_to_object_warehouse.end();
  const auto & tag_warehouse = vector_tag_to_object_warehouse.find(tag_id);

  if (tag_warehouse != house_end)
    return tag_warehouse->second;
  else
    vector_tag_to_object_warehouse[tag_id];

  // Now add actual moose objects into warehouse
  const auto & objects = MooseObjectWarehouse<T>::getObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getVectorTags({});
    for (auto & tag : tags)
    {
      if (tag == tag_id)
      {
        // Tag based storage
        vector_tag_to_object_warehouse[tag_id].addObject(object, tid);
        break;
      }
    }
  }

  return vector_tag_to_object_warehouse[tag_id];
}

template <typename T>
MooseObjectWarehouse<T> &
MooseObjectTagWarehouse<T>::getMatrixTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  auto & matrix_tag_to_object_warehouse = _matrix_tag_to_object_warehouse[tid];

  const auto & house_end = matrix_tag_to_object_warehouse.end();
  const auto & tag_warehouse = matrix_tag_to_object_warehouse.find(tag_id);

  if (tag_warehouse != house_end)
    return tag_warehouse->second;
  else
    matrix_tag_to_object_warehouse[tag_id];

  // Add moose objects to matrix-tag warehouse
  const auto & objects = MooseObjectWarehouse<T>::getObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getMatrixTags({});
    for (auto & tag : tags)
    {
      if (tag == tag_id)
      {
        // Tag based storage
        matrix_tag_to_object_warehouse[tag_id].addObject(object, tid);

        break;
      }
    }
  }

  return matrix_tag_to_object_warehouse[tag_id];
}

template <typename T>
MooseObjectWarehouse<T> &
MooseObjectTagWarehouse<T>::getVectorTagsObjectWarehouse(const std::set<TagID> & v_tags,
                                                         THREAD_ID tid)
{
  // std::map is not thread-safe for writes
  auto & vector_tags_to_object_warehouse = _vector_tags_to_object_warehouse[tid];

  const auto & house_end = vector_tags_to_object_warehouse.end();
  const auto & tags_warehouse = vector_tags_to_object_warehouse.find(v_tags);

  if (tags_warehouse != house_end)
    return tags_warehouse->second;
  else
    vector_tags_to_object_warehouse[v_tags];

  // Add moose objects to vector-tags warehouse
  const auto & objects = MooseObjectWarehouse<T>::getObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getVectorTags({});
    const auto & tags_end = tags.end();
    for (auto & v_tag : v_tags)
    {
      const auto & tag_found = tags.find(v_tag);
      // Object contains at least one of required tags
      if (tag_found != tags_end)
      {
        // std::vector<Tag> based storage
        vector_tags_to_object_warehouse[v_tags].addObject(object, tid);
        // Then we should work for next object
        break;
      }
    }
  }

  return vector_tags_to_object_warehouse[v_tags];
}

template <typename T>
MooseObjectWarehouse<T> &
MooseObjectTagWarehouse<T>::getMatrixTagsObjectWarehouse(const std::set<TagID> & m_tags,
                                                         THREAD_ID tid)
{
  auto & matrix_tags_to_object_warehouse = _matrix_tags_to_object_warehouse[tid];

  const auto & house_end = matrix_tags_to_object_warehouse.end();
  const auto & tags_warehouse = matrix_tags_to_object_warehouse.find(m_tags);

  if (tags_warehouse != house_end)
    return tags_warehouse->second;
  else
    matrix_tags_to_object_warehouse[m_tags];

  const auto & objects = MooseObjectWarehouse<T>::getObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getMatrixTags({});
    const auto & tags_end = tags.end();
    for (auto & m_tag : m_tags)
    {
      const auto & tag_found = tags.find(m_tag);
      // Object contains at least one of required tags
      if (tag_found != tags_end)
      {
        // std::vector<Tag> based storage
        matrix_tags_to_object_warehouse[m_tags].addObject(object, tid);
        // Then we should work for next object
        break;
      }
    }
  }

  return matrix_tags_to_object_warehouse[m_tags];
}
