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

  return getVectorPostprocessorHelper(vpp_name, vector_name, true);
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
                                       const std::string & vector_name)
{
  _supplied_items.emplace(vpp_name + "::" + vector_name);

  return getVectorPostprocessorHelper(vpp_name, vector_name, true);
}

VectorPostprocessorValue &
VectorPostprocessorData::getVectorPostprocessorHelper(const VectorPostprocessorName & vpp_name,
                                                      const std::string & vector_name,
                                                      bool get_current)
{
  // Intentional use of RHS brackets on a std::map to do a retrieve or insert
  auto & vec_storage = _vpp_data[vpp_name];

  // lambda for doing comparison on name (i.e., first item in pair)
  auto comp = [&vector_name](std::pair<std::string, VectorPostprocessorState> & pair) {
    return pair.first == vector_name;
  };

  // Search for the vector, if it is not located create the entry in the storage
  auto iter = std::find_if(vec_storage._values.rbegin(), vec_storage._values.rend(), comp);
  if (iter == vec_storage._values.rend())
  {
    vec_storage._values.emplace_back(
        std::pair<std::string, VectorPostprocessorState>(vector_name, VectorPostprocessorState()));
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
  for (const auto & vec_pair : _vpp_data)
    if (!vec_pair.second._contains_complete_history)
      for (const auto & vec_it : vec_pair.second._values)
        vec_it.second.old->swap(*vec_it.second.current);
}
