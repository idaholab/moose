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

  params.addClassDescription(
      "Outputs the values of an arbitrary user-specified set of "
      "vectorpostprocessors as a combined vector in the order specified by the user");

  // The value from this VPP is naturally already on every processor
  // TODO: Make this not the case!  See #11415
  params.set<bool>("_auto_broadcast") = false;

  return params;
}

CombinedVectorPostprocessor::CombinedVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters)
{
  // Get all the names of the vectors to combine them
  std::vector<VectorPostprocessorName> vpps_names(
      getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors"));
  std::vector<std::vector<std::string>> vpp_vectors(vpps_names.size());
  for (const auto i : index_range(vpps_names))
  {
    const auto & vpp_name = vpps_names[i];
    const auto & vpp = _vpp_fe_problem.getVectorPostprocessorObjectByName(vpp_name);
    for (const auto & vec_name : vpp.getVectorNames())
    {
      vpp_vectors[i].push_back(vec_name);
      // Declare vectors
      _vpp_vecs.push_back(&this->declareVector(vpp_name + "_" + vec_name));
    }
  }

  // Grab a reference to all the values
  for (const auto i : index_range(vpps_names))
    for (const auto & vec_name : vpp_vectors[i])
    {
      std::cout << vpps_names[i] << " " << vec_name << std::endl;
      _vectorpostprocessor_values.push_back(&getVectorPostprocessorValueByName(vpps_names[i], vec_name));
    }
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
  // The vectors are already ordered
  for (const auto i : index_range(_vectorpostprocessor_values))
    *(_vpp_vecs[i]) = *(_vectorpostprocessor_values[i]);
}
