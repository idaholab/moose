//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * User Object for testing triple index parsing
 */
class ReadTripleIndex : public GeneralUserObject
{
public:
  static InputParameters validParams();

  ReadTripleIndex(const InputParameters & params);

  virtual void initialize(){};
  virtual void execute();
  virtual void finalize(){};

protected:
  const std::vector<std::vector<std::vector<Real>>> & _real_tri;
  const std::vector<std::vector<std::vector<Real>>> & _real_tri_empty_sub;
  const std::vector<std::vector<std::vector<Real>>> & _real_tri_empty_subsub;
  const std::vector<std::vector<std::vector<Real>>> & _real_tri_empty_subs;
  const std::vector<std::vector<std::vector<Real>>> & _real_tri_empty_subsubs;
  const std::vector<std::vector<std::vector<Real>>> & _real_tri_all_empty;
  const std::vector<std::vector<std::vector<unsigned int>>> & _uint_tri;
  const std::vector<std::vector<std::vector<int>>> & _int_tri;
  const std::vector<std::vector<std::vector<long>>> & _long_tri;
  const std::vector<std::vector<std::vector<SubdomainID>>> & _subid_tri;
  const std::vector<std::vector<std::vector<BoundaryID>>> & _bid_tri;

  const std::vector<std::vector<std::vector<std::string>>> & _str_tri;
  const std::vector<std::vector<std::vector<FileName>>> & _file_tri;
  const std::vector<std::vector<std::vector<FileNameNoExtension>>> & _file_no_tri;

  const std::vector<std::vector<std::vector<MeshFileName>>> & _mesh_file_tri;
  const std::vector<std::vector<std::vector<SubdomainName>>> & _subdomain_name_tri;
  const std::vector<std::vector<std::vector<BoundaryName>>> & _boundary_name_tri;
  const std::vector<std::vector<std::vector<FunctionName>>> & _function_name_tri;
  const std::vector<std::vector<std::vector<UserObjectName>>> & _userobject_name_tri;
  const std::vector<std::vector<std::vector<IndicatorName>>> & _indicator_name_tri;

  const std::vector<std::vector<std::vector<MarkerName>>> & _marker_name_tri;
  const std::vector<std::vector<std::vector<MultiAppName>>> & _multiapp_name_tri;
  const std::vector<std::vector<std::vector<PostprocessorName>>> & _postprocessor_name_tri;
  const std::vector<std::vector<std::vector<VectorPostprocessorName>>> &
      _vector_postprocessor_name_tri;
  const std::vector<std::vector<std::vector<OutputName>>> & _output_name_tri;
  const std::vector<std::vector<std::vector<MaterialPropertyName>>> & _material_property_name_tri;
  const std::vector<std::vector<std::vector<MaterialName>>> & _material_name_tri;
  const std::vector<std::vector<std::vector<MooseFunctorName>>> & _moose_functor_name_tri;
  const std::vector<std::vector<std::vector<DistributionName>>> & _distribution_name;
  const std::vector<std::vector<std::vector<SamplerName>>> & _sampler_name;

  template <typename T>
  void
  TripleIndexNumberVectorChecker(const std::vector<std::vector<std::vector<T>>> triple_index_vector,
                                 const std::string vector_name,
                                 const std::vector<std::vector<std::vector<Real>>> reference_values,
                                 const bool index_flip = false,
                                 const bool flip_direction = false,
                                 const Real scale_factor = 1.0,
                                 const Real offset_value = 0.0);

  template <typename T>
  void TripleIndexStringVectorChecker(
      const T triple_index_vector,
      const std::string vector_name,
      const std::string prefix,
      const std::vector<std::vector<std::vector<Real>>> reference_values);
};

void
ReadTripleIndex::execute()
{
}
