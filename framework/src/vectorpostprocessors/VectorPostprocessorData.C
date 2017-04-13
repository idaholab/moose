/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "VectorPostprocessorData.h"
#include "FEProblem.h"

VectorPostprocessorData::VectorPostprocessorData(FEProblemBase & fe_problem)
  : Restartable("values", "VectorPostprocessorData", fe_problem, 0)
{
}

bool
VectorPostprocessorData::hasVectorPostprocessor(const std::string & name)
{
  return (_values.find(name) != _values.end());
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
  // Intentional use of RHS brackets on a std::map to do a multilevel retrieve or insert
  auto & vec_struct = _values[vpp_name][vector_name];

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
  return _values.find(vpp_name) != _values.end();
}

const std::map<std::string, VectorPostprocessorData::VectorPostprocessorState> &
VectorPostprocessorData::vectors(const std::string & vpp_name) const
{
  auto vec_pair = _values.find(vpp_name);
  mooseAssert(vec_pair != _values.end(), "No vectors found for vpp_name: " << vpp_name);

  return vec_pair->second;
}

void
VectorPostprocessorData::copyValuesBack()
{
  for (const auto & it : _values)
    for (const auto & vec_it : it.second)
      vec_it.second.old->swap(*vec_it.second.current);
}
