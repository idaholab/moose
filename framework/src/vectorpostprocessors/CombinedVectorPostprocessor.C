//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CombinedVectorPostprocessor.h"
#include "VectorPostprocessorInterface.h"
#include "VectorPostprocessor.h"

registerMooseObject("MooseApp", CombinedVectorPostprocessor);

InputParameters
CombinedVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addRequiredParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors", "The vectorpostprocessors whose vector values are to be combined");
  params.addParam<std::vector<std::vector<std::string>>>(
      "vectors", "Vectors to combine from each vectorpostprocessor");
  params.addParam<Real>("vector_filler_value",
                        0,
                        "Which value to use to fill the smaller vectors (any smaller than the "
                        "largest vector) if the vectors are not the same size");
  params.addClassDescription(
      "Outputs the values of an arbitrary user-specified set of "
      "vectorpostprocessors as a combined vector in the order specified by the user");

  // If the vectorpostprocessors are already broadcasted, this VPP should not need to
  // broadcast on its own
  params.set<bool>("_auto_broadcast") = false;

  return params;
}

CombinedVectorPostprocessor::CombinedVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), _filler_value(getParam<Real>("vector_filler_value"))
{
  // Get all the names of the vectors to combine them
  std::vector<VectorPostprocessorName> vpps_names(
      getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors"));
  std::vector<std::vector<std::string>> vpp_vectors(vpps_names.size());
  bool vecs_from_param = isParamValid("vectors");
  if (!vecs_from_param)
    for (const auto i : index_range(vpps_names))
    {
      const auto & vpp_name = vpps_names[i];
      const auto & vpp = _vpp_fe_problem.getVectorPostprocessorObjectByName(vpp_name);
      for (const auto & vec_name : vpp.getVectorNames())
        vpp_vectors[i].push_back(vec_name);
    }
  if (vecs_from_param)
  {
    vpp_vectors = getParam<std::vector<std::vector<std::string>>>("vectors");
    if (vpp_vectors.size() != vpps_names.size())
      paramError(
          "vectors",
          "Outer vector size should be the same size as the 'vectorpostprocessors' parameter");
  }

  // Declare vectors
  for (const auto i : index_range(vpp_vectors))
    for (const auto j : index_range(vpp_vectors[i]))
      _vpp_vecs.push_back(&this->declareVector(vpps_names[i] + "_" + vpp_vectors[i][j]));

  // Grab a reference to all the values
  for (const auto i : index_range(vpps_names))
    for (const auto & vec_name : vpp_vectors[i])
      // This will error if the vectors don't exist in the VPP
      _vectorpostprocessor_values.push_back(
          &getVectorPostprocessorValueByName(vpps_names[i], vec_name));
}

void
CombinedVectorPostprocessor::initialize()
{
  for (auto & vec : _vpp_vecs)
    vec->clear();
}

void
CombinedVectorPostprocessor::execute()
{
  unsigned long max_size = 0;
  // The vectors are already ordered
  for (const auto i : index_range(_vectorpostprocessor_values))
  {
    *(_vpp_vecs[i]) = *(_vectorpostprocessor_values[i]);
    max_size = std::max(max_size, _vectorpostprocessor_values[i]->size());
  }

  // Resize to the max size
  for (const auto i : index_range(_vectorpostprocessor_values))
    _vpp_vecs[i]->resize(max_size);

  // Fill with the filler value
  for (const auto i : index_range(_vectorpostprocessor_values))
    for (const auto j : make_range(_vectorpostprocessor_values[i]->size(), max_size))
      (*(_vpp_vecs[i]))[j] = _filler_value;
}
