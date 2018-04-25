//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorData.h"
#include "FEProblem.h"

VectorPostprocessorData::VectorPostprocessorData(FEProblemBase & fe_problem)
  : Restartable(fe_problem.getMooseApp(), "values", "VectorPostprocessorData", 0)
{
}

void
VectorPostprocessorData::init(const std::string & /*name*/)
{
}

bool
VectorPostprocessorData::containsCompleteHistory(const std::string & name) const
{
  auto it = _vpp_data.find(name);
  mooseAssert(it != _vpp_data.end(), std::string("VectorPostprocessor ") + name + " not found!");

  return it->second._contains_complete_history;
}

bool
VectorPostprocessorData::hasVectorPostprocessor(const std::string & name)
{
  return (_vpp_data.find(name) != _vpp_data.end());
}

VectorPostprocessorValue &
VectorPostprocessorData::getVectorPostprocessorValue(const VectorPostprocessorName & vpp_name,
                                                     const std::string & vector_name)
{
  _requested_items.emplace(vpp_name + "::" + vector_name);

  return getVectorPostprocessorHelper(vpp_name, vector_name);
}

VectorPostprocessorValue &
VectorPostprocessorData::getVectorPostprocessorValueOld(const VectorPostprocessorName & vpp_name,
                                                        const std::string & vector_name)
{
  _requested_items.emplace(vpp_name + "::" + vector_name);

  return getVectorPostprocessorHelper(vpp_name, vector_name, false);
}

VectorPostprocessorValue &
VectorPostprocessorData::declareVector(const std::string & vpp_name,
                                       const std::string & vector_name,
                                       bool contains_complete_history)
{
  _supplied_items.emplace(vpp_name + "::" + vector_name);

  return getVectorPostprocessorHelper(vpp_name, vector_name, true, contains_complete_history);
}

VectorPostprocessorValue &
VectorPostprocessorData::getVectorPostprocessorHelper(const VectorPostprocessorName & vpp_name,
                                                      const std::string & vector_name,
                                                      bool get_current,
                                                      bool contains_complete_history)
{
  // Retrieve or create the data structure for this VPP
  auto vec_it_pair = _vpp_data.emplace(
      std::piecewise_construct, std::forward_as_tuple(vpp_name), std::forward_as_tuple());
  auto & vec_storage = vec_it_pair.first->second;

  // If the VPP is declaring a vector, see if complete history is needed. Note: This parameter
  // is constant and applies to _all_ declared vectors.
  vec_storage._contains_complete_history |= contains_complete_history;

  // Keep track of whether an old vector is needed for copying back later.
  if (!get_current)
    vec_storage._needs_old = true;

  // lambda for doing comparison on name (i.e., first item in pair)
  auto comp = [&vector_name](std::pair<std::string, VectorPostprocessorState> & pair) {
    return pair.first == vector_name;
  };
  // Search for the vector, if it is not located create the entry in the storage
  auto iter = std::find_if(vec_storage._values.rbegin(), vec_storage._values.rend(), comp);
  if (iter == vec_storage._values.rend())
  {
    vec_storage._values.emplace_back(
        std::piecewise_construct, std::forward_as_tuple(vector_name), std::forward_as_tuple());
    iter = vec_storage._values.rbegin();
  }

  auto & vec_struct = iter->second;
  if (!vec_struct.current)
  {
    mooseAssert(!vec_struct.old, "Uninitialized pointers in VectorPostprocessor Data");
    vec_struct.current = &declareRestartableDataWithObjectName<VectorPostprocessorValue>(
        vpp_name + "_" + vector_name, "values");
    vec_struct.old = &declareRestartableDataWithObjectName<VectorPostprocessorValue>(
        vpp_name + "_" + vector_name, "values_old");
  }

  return get_current ? *vec_struct.current : *vec_struct.old;
}

bool
VectorPostprocessorData::hasVectors(const std::string & vpp_name) const
{
  auto it_pair = _vpp_data.find(vpp_name);
  if (it_pair != _vpp_data.end())
    return !it_pair->second._values.empty();

  return false;
}

const std::vector<std::pair<std::string, VectorPostprocessorData::VectorPostprocessorState>> &
VectorPostprocessorData::vectors(const std::string & vpp_name) const
{
  auto it_pair = _vpp_data.find(vpp_name);
  mooseAssert(it_pair != _vpp_data.end(), "No vectors found for vpp_name: " << vpp_name);

  return it_pair->second._values;
}

void
VectorPostprocessorData::copyValuesBack()
{
  /**
   * Depending on the needs of this VPP, we'll perform
   * different actions on the vectors as time is advanced.
   *
   *   Need Old | Preserve History | Action
   *  --------------------------------------
   *    False   |      False       |  None
   *    False   |      True        |  None
   *    True    |      False       |  Swap
   *    True    |      True        |  Copy
   */
  for (const auto & vec_pair : _vpp_data)
  {
    bool needs_old = vec_pair.second._needs_old;
    bool preserve_history = vec_pair.second._contains_complete_history;

    if (!needs_old)
      continue;

    for (const auto & vec_it : vec_pair.second._values)
      if (preserve_history)
        *vec_it.second.old = *vec_it.second.current;
      else
        vec_it.second.old->swap(*vec_it.second.current);
  }
}

VectorPostprocessorData::VectorPostprocessorVectors::VectorPostprocessorVectors()
  : _contains_complete_history(false), _needs_old(false)
{
}
