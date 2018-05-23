//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstraintTagWarehouse.h"

ConstraintTagWarehouse::ConstraintTagWarehouse(bool threaded /*=false*/)
  : ConstraintWarehouse(),
    _num_threads(threaded ? libMesh::n_threads() : 1),
    _vector_tag_to_object_warehouse(_num_threads),
    _vector_tags_to_object_warehouse(_num_threads),
    _matrix_tag_to_object_warehouse(_num_threads),
    _matrix_tags_to_object_warehouse(_num_threads)
{
}

void
ConstraintTagWarehouse::updateActive(THREAD_ID tid)
{
  ConstraintWarehouse::updateActive(tid);

  for (auto & it : _vector_tag_to_object_warehouse[tid])
    it.second.updateActive(tid);

  for (auto & it : _vector_tags_to_object_warehouse[tid])
    it.second.updateActive(tid);

  for (auto & it : _matrix_tag_to_object_warehouse[tid])
    it.second.updateActive(tid);

  for (auto & it : _matrix_tags_to_object_warehouse[tid])
    it.second.updateActive(tid);
}

ConstraintWarehouse &
ConstraintTagWarehouse::getVectorTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  auto & vector_tag_to_object_warehouse = _vector_tag_to_object_warehouse[tid];

  const auto & house_end = vector_tag_to_object_warehouse.end();
  const auto & tag_warehouse = vector_tag_to_object_warehouse.find(tag_id);

  if (tag_warehouse != house_end)
    return tag_warehouse->second;
  else
    vector_tag_to_object_warehouse[tag_id];

  // Now add actual moose objects into warehouse
  const auto & objects = ConstraintWarehouse::getActiveObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getVectorTags();
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

ConstraintWarehouse &
ConstraintTagWarehouse::getMatrixTagObjectWarehouse(TagID tag_id, THREAD_ID tid)
{
  auto & matrix_tag_to_object_warehouse = _matrix_tag_to_object_warehouse[tid];

  const auto & house_end = matrix_tag_to_object_warehouse.end();
  const auto & tag_warehouse = matrix_tag_to_object_warehouse.find(tag_id);

  if (tag_warehouse != house_end)
    return tag_warehouse->second;
  else
    matrix_tag_to_object_warehouse[tag_id];

  // Add moose objects to matrix-tag warehouse
  const auto & objects = ConstraintWarehouse::getActiveObjects(tid);
  for (auto & object : objects)
  {
    auto & tags = object->getMatrixTags();
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

ConstraintWarehouse &
ConstraintTagWarehouse::getVectorTagsObjectWarehouse(const std::set<TagID> & v_tags, THREAD_ID tid)
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
  const auto & objects = ConstraintWarehouse::getActiveObjects(tid);
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
        vector_tags_to_object_warehouse[v_tags].addObject(object, tid);
        // Then we should work for next object
        break;
      }
    }
  }

  return vector_tags_to_object_warehouse[v_tags];
}

ConstraintWarehouse &
ConstraintTagWarehouse::getMatrixTagsObjectWarehouse(const std::set<TagID> & m_tags, THREAD_ID tid)
{
  auto & matrix_tags_to_object_warehouse = _matrix_tags_to_object_warehouse[tid];

  const auto & house_end = matrix_tags_to_object_warehouse.end();
  const auto & tags_warehouse = matrix_tags_to_object_warehouse.find(m_tags);

  if (tags_warehouse != house_end)
    return tags_warehouse->second;
  else
    matrix_tags_to_object_warehouse[m_tags];

  const auto & objects = ConstraintWarehouse::getActiveObjects(tid);
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
        matrix_tags_to_object_warehouse[m_tags].addObject(object, tid);
        // Then we should work for next object
        break;
      }
    }
  }

  return matrix_tags_to_object_warehouse[m_tags];
}
