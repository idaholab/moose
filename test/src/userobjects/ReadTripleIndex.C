//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReadTripleIndex.h"
#include <sstream>

registerMooseObject("MooseTestApp", ReadTripleIndex);

InputParameters
ReadTripleIndex::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::vector<std::vector<std::vector<Real>>>>(
      "real_tri", "A triple-indexed vector of real numbers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<Real>>>>(
      "real_tri_empty_sub", "A triple-indexed vector of real numbers with an empty subvector.");
  params.addRequiredParam<std::vector<std::vector<std::vector<Real>>>>(
      "real_tri_empty_subsub",
      "A triple-indexed vector of real numbers with an empty sub-subvector.");
  params.addRequiredParam<std::vector<std::vector<std::vector<Real>>>>(
      "real_tri_empty_subs", "A triple-indexed vector of real numbers with an empty subvectors.");
  params.addRequiredParam<std::vector<std::vector<std::vector<Real>>>>(
      "real_tri_empty_subsubs",
      "A triple-indexed vector of real numbers with empty sub-subvectors.");
  params.addRequiredParam<std::vector<std::vector<std::vector<Real>>>>(
      "real_tri_all_empty", "An empty triple-indexed vector of real numbers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<unsigned int>>>>(
      "uint_tri", "A triple-indexed vector of unsigned integers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<int>>>>(
      "int_tri", "A triple-indexed vector of integers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<long>>>>(
      "long_tri", "A triple-indexed vector of long integers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<SubdomainID>>>>(
      "subid_tri", "A triple-indexed vector of SubdomainID.");
  params.addRequiredParam<std::vector<std::vector<std::vector<BoundaryID>>>>(
      "bid_tri", "A triple-indexed vector of BoundaryID.");
  params.addRequiredParam<std::vector<std::vector<std::vector<std::string>>>>(
      "str_tri", "A triple-indexed vector of std::string.");
  params.addRequiredParam<std::vector<std::vector<std::vector<FileName>>>>(
      "file_tri", "A triple-indexed vector of FileName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<FileNameNoExtension>>>>(
      "file_no_tri", "A triple-indexed vector of FileNameNoExtension.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MeshFileName>>>>(
      "mesh_file_tri", "A triple-indexed vector of MeshFileName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<SubdomainName>>>>(
      "subdomain_name_tri", "A triple-indexed vector of SubdomainName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<BoundaryName>>>>(
      "boundary_name_tri", "A triple-indexed vector of BoundaryName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<FunctionName>>>>(
      "function_name_tri", "A triple-indexed vector of FunctionName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<UserObjectName>>>>(
      "userobject_name_tri", "A triple-indexed vector of UserObjectName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<IndicatorName>>>>(
      "indicator_name_tri", "A triple-indexed vector of IndicatorName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MarkerName>>>>(
      "marker_name_tri", "A triple-indexed vector of MarkerName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MultiAppName>>>>(
      "multiapp_name_tri", "A triple-indexed vector of MultiAppName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<PostprocessorName>>>>(
      "postprocessor_name_tri", "A triple-indexed vector of PostprocessorName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<VectorPostprocessorName>>>>(
      "vector_postprocessor_name_tri", "A triple-indexed vector of VectorPostprocessorName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<OutputName>>>>(
      "output_name_tri", "A triple-indexed vector of OutputName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MaterialPropertyName>>>>(
      "material_property_name_tri", "A triple-indexed vector of MaterialPropertyName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MaterialName>>>>(
      "material_name_tri", "A triple-indexed vector of MaterialName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MooseFunctorName>>>>(
      "moose_functor_name_tri", "A triple-indexed vector of MooseFunctorName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<DistributionName>>>>(
      "distribution_name_tri", "A triple-indexed vector of DistributionName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<SamplerName>>>>(
      "sampler_name_tri", "A triple-indexed vector of SamplerName.");
  return params;
}

ReadTripleIndex::ReadTripleIndex(const InputParameters & params)
  : GeneralUserObject(params),
    _real_tri(getParam<std::vector<std::vector<std::vector<Real>>>>("real_tri")),
    _real_tri_empty_sub(
        getParam<std::vector<std::vector<std::vector<Real>>>>("real_tri_empty_sub")),
    _real_tri_empty_subsub(
        getParam<std::vector<std::vector<std::vector<Real>>>>("real_tri_empty_subsub")),
    _real_tri_empty_subs(
        getParam<std::vector<std::vector<std::vector<Real>>>>("real_tri_empty_subs")),
    _real_tri_empty_subsubs(
        getParam<std::vector<std::vector<std::vector<Real>>>>("real_tri_empty_subsubs")),
    _real_tri_all_empty(
        getParam<std::vector<std::vector<std::vector<Real>>>>("real_tri_all_empty")),
    _uint_tri(getParam<std::vector<std::vector<std::vector<unsigned int>>>>("uint_tri")),
    _int_tri(getParam<std::vector<std::vector<std::vector<int>>>>("int_tri")),
    _long_tri(getParam<std::vector<std::vector<std::vector<long>>>>("long_tri")),
    _subid_tri(getParam<std::vector<std::vector<std::vector<SubdomainID>>>>("subid_tri")),
    _bid_tri(getParam<std::vector<std::vector<std::vector<BoundaryID>>>>("bid_tri")),
    _str_tri(getParam<std::vector<std::vector<std::vector<std::string>>>>("str_tri")),
    _file_tri(getParam<std::vector<std::vector<std::vector<FileName>>>>("file_tri")),
    _file_no_tri(
        getParam<std::vector<std::vector<std::vector<FileNameNoExtension>>>>("file_no_tri")),
    _mesh_file_tri(getParam<std::vector<std::vector<std::vector<MeshFileName>>>>("mesh_file_tri")),
    _subdomain_name_tri(
        getParam<std::vector<std::vector<std::vector<SubdomainName>>>>("subdomain_name_tri")),
    _boundary_name_tri(
        getParam<std::vector<std::vector<std::vector<BoundaryName>>>>("boundary_name_tri")),
    _function_name_tri(
        getParam<std::vector<std::vector<std::vector<FunctionName>>>>("function_name_tri")),
    _userobject_name_tri(
        getParam<std::vector<std::vector<std::vector<UserObjectName>>>>("userobject_name_tri")),
    _indicator_name_tri(
        getParam<std::vector<std::vector<std::vector<IndicatorName>>>>("indicator_name_tri")),
    _marker_name_tri(
        getParam<std::vector<std::vector<std::vector<MarkerName>>>>("marker_name_tri")),
    _multiapp_name_tri(
        getParam<std::vector<std::vector<std::vector<MultiAppName>>>>("multiapp_name_tri")),
    _postprocessor_name_tri(getParam<std::vector<std::vector<std::vector<PostprocessorName>>>>(
        "postprocessor_name_tri")),
    _vector_postprocessor_name_tri(
        getParam<std::vector<std::vector<std::vector<VectorPostprocessorName>>>>(
            "vector_postprocessor_name_tri")),
    _output_name_tri(
        getParam<std::vector<std::vector<std::vector<OutputName>>>>("output_name_tri")),
    _material_property_name_tri(
        getParam<std::vector<std::vector<std::vector<MaterialPropertyName>>>>(
            "material_property_name_tri")),
    _material_name_tri(
        getParam<std::vector<std::vector<std::vector<MaterialName>>>>("material_name_tri")),
    _moose_functor_name_tri(getParam<std::vector<std::vector<std::vector<MooseFunctorName>>>>(
        "moose_functor_name_tri")),
    _distribution_name(
        getParam<std::vector<std::vector<std::vector<DistributionName>>>>("distribution_name_tri")),
    _sampler_name(getParam<std::vector<std::vector<std::vector<SamplerName>>>>("sampler_name_tri"))
{
  const std::vector<std::vector<std::vector<Real>>> reference_values = {
      {{1.1}, {2.1, 2.2, 2.3}, {3.1, 3.2}},
      {{11.1, 11.2}},
      {{21.1, 21.2}, {22.1}, {23.1, 23.2, 23.3}}};
  const std::vector<std::vector<std::vector<Real>>> reference_values_empty_sub = {
      {{1.1}, {2.1, 2.2, 2.3}, {3.1, 3.2}}, {}, {{21.1, 21.2}, {22.1}, {23.1, 23.2, 23.3}}};
  const std::vector<std::vector<std::vector<Real>>> reference_values_empty_subsub = {
      {{1.1}, {2.1, 2.2, 2.3}, {3.1, 3.2}}, {{11.1, 11.2}}, {{}, {22.1}, {23.1, 23.2, 23.3}}};
  const std::vector<std::vector<std::vector<Real>>> reference_values_empty_subs = {
      {}, {}, {{21.1, 21.2}, {22.1}, {23.1, 23.2, 23.3}}};
  const std::vector<std::vector<std::vector<Real>>> reference_values_empty_subsubs = {
      {{}, {}, {3.1, 3.2}}, {}, {{21.1, 21.2}, {22.1}, {23.1, 23.2, 23.3}}};
  const std::vector<std::vector<std::vector<Real>>> reference_values_all_empty = {{}, {{}, {}}, {}};

  // check real and special cases with real
  TripleIndexNumberVectorChecker(_real_tri, "real_tri", reference_values);
  TripleIndexNumberVectorChecker(
      _real_tri_empty_sub, "real_tri_empty_sub", reference_values_empty_sub);
  TripleIndexNumberVectorChecker(
      _real_tri_empty_subsub, "real_tri_empty_subsub", reference_values_empty_subsub);
  TripleIndexNumberVectorChecker(
      _real_tri_empty_subs, "real_tri_empty_subs", reference_values_empty_subs);
  TripleIndexNumberVectorChecker(
      _real_tri_empty_subsubs, "real_tri_empty_subsubs", reference_values_empty_subsubs);
  TripleIndexNumberVectorChecker(
      _real_tri_all_empty, "real_tri_all_empty", reference_values_all_empty);

  // check unsigned int
  TripleIndexNumberVectorChecker(_uint_tri, "uint_tri", reference_values, false, false, 10.0);

  // check int
  TripleIndexNumberVectorChecker(_int_tri, "int_tri", reference_values, true, true, 10.0);

  // check long
  TripleIndexNumberVectorChecker(_long_tri, "long_tri", reference_values, true, false, 10.0);

  // check SubdomainID
  TripleIndexNumberVectorChecker(
      _subid_tri, "subid_tri", reference_values, false, false, 10.0, 30.0);

  // check BoundaryID
  TripleIndexNumberVectorChecker(_bid_tri, "bid_tri", reference_values, false, false, 10.0, 35.0);

  // check std::string
  TripleIndexStringVectorChecker(_str_tri, "str_tri", "string", reference_values);

  // check FileName
  TripleIndexStringVectorChecker(_file_tri, "file_tri", "file", reference_values);

  // check FileNameNoExtension
  TripleIndexStringVectorChecker(_file_no_tri, "file_no_tri", "file_no", reference_values);

  // check MeshFileName
  TripleIndexStringVectorChecker(_mesh_file_tri, "mesh_file_tri", "mesh_file", reference_values);

  // check SubdomainName
  TripleIndexStringVectorChecker(
      _subdomain_name_tri, "subdomain_name_tri", "subdomain_name", reference_values);

  // check BoundaryName
  TripleIndexStringVectorChecker(
      _boundary_name_tri, "boundary_name_tri", "boundary_name", reference_values);

  // check FunctionName
  TripleIndexStringVectorChecker(
      _function_name_tri, "function_name_tri", "function_name", reference_values);

  // check UserObjectName
  TripleIndexStringVectorChecker(
      _userobject_name_tri, "userobject_name_tri", "userobject_name", reference_values);

  // check IndicatorName
  TripleIndexStringVectorChecker(
      _indicator_name_tri, "indicator_name_tri", "indicator_name", reference_values);

  // check MarkerName
  TripleIndexStringVectorChecker(
      _marker_name_tri, "marker_name_tri", "marker_name", reference_values);

  // check MultiAppName
  TripleIndexStringVectorChecker(
      _multiapp_name_tri, "multiapp_name_tri", "multiapp_name", reference_values);

  // check PostprocessorName
  TripleIndexStringVectorChecker(
      _postprocessor_name_tri, "postprocessor_name_tri", "postprocessor_name", reference_values);

  // check VectorPostprocessorName
  TripleIndexStringVectorChecker(_vector_postprocessor_name_tri,
                                 "vector_postprocessor_name_tri",
                                 "vector_postprocessor_name",
                                 reference_values);

  // check OutputName
  TripleIndexStringVectorChecker(
      _output_name_tri, "output_name_tri", "output_name", reference_values);

  // check MaterialPropertyName
  TripleIndexStringVectorChecker(_material_property_name_tri,
                                 "material_property_name_tri",
                                 "material_property_name",
                                 reference_values);

  // check MaterialName
  TripleIndexStringVectorChecker(
      _material_name_tri, "material_name_tri", "material_name", reference_values);

  // check MooseFunctorName
  TripleIndexStringVectorChecker(
      _moose_functor_name_tri, "moose_functor_name_tri", "moose_functor_name", reference_values);

  // check DistributionName
  TripleIndexStringVectorChecker(
      _distribution_name, "distribution_name_tri", "distribution_name", reference_values);

  // check SamplerName
  TripleIndexStringVectorChecker(
      _sampler_name, "sampler_name_tri", "sampler_name", reference_values);
}

template <typename T>
void
ReadTripleIndex::TripleIndexNumberVectorChecker(
    const std::vector<std::vector<std::vector<T>>> triple_index_vector,
    const std::string vector_name,
    const std::vector<std::vector<std::vector<Real>>> reference_values,
    const bool index_flip,
    const bool flip_direction,
    const Real scale_factor,
    const Real offset_value)
{
  if (triple_index_vector.size() != reference_values.size())
    mooseError("Error reading ", vector_name, ".");
  for (unsigned int i = 0; i < triple_index_vector.size(); i++)
  {
    if (triple_index_vector[i].size() != reference_values[i].size())
      mooseError("Error reading ", vector_name, ".");
    for (unsigned int j = 0; j < triple_index_vector[i].size(); j++)
    {
      if (triple_index_vector[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading ", vector_name, ".");
      for (unsigned int k = 0; k < triple_index_vector[i][j].size(); k++)
        if (triple_index_vector[i][j][k] !=
            static_cast<T>((reference_values[i][j][k] * scale_factor + offset_value) *
                           (index_flip
                                ? (flip_direction ? ((i % 2 ? 1.0 : -1.0) * (j % 2 ? 1.0 : -1.0))
                                                  : ((i % 2 ? 1.0 : -1.0) * (j % 2 ? -1.0 : 1.0)))
                                : 1.0)))
          mooseError("Error reading ", vector_name, ".");
    }
  }
}

template <typename T>
void
ReadTripleIndex::TripleIndexStringVectorChecker(
    const T triple_index_vector,
    const std::string vector_name,
    const std::string prefix,
    const std::vector<std::vector<std::vector<Real>>> reference_values)
{
  if (triple_index_vector.size() != reference_values.size())
    mooseError("Error reading ", vector_name, ".");
  for (unsigned int i = 0; i < triple_index_vector.size(); i++)
  {
    if (triple_index_vector[i].size() != reference_values[i].size())
      mooseError("Error reading ", vector_name, ".");
    for (unsigned int j = 0; j < triple_index_vector[i].size(); j++)
    {
      if (triple_index_vector[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading ", vector_name, ".");
      for (unsigned int k = 0; k < triple_index_vector[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << prefix << i << j << k;
        if (triple_index_vector[i][j][k] != ss.str())
          mooseError("Error reading ", vector_name, ".");
      }
    }
  }
}
