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
 * User Object for testing double index parsing
 */
class ReadDoubleIndex : public GeneralUserObject
{
public:
  static InputParameters validParams();

  ReadDoubleIndex(const InputParameters & params);

  virtual void initialize(){};
  virtual void execute();
  virtual void finalize(){};

protected:
  const std::vector<std::vector<Real>> & _real_di;
  const std::vector<std::vector<unsigned int>> & _uint_di;
  const std::vector<std::vector<int>> & _int_di;
  const std::vector<std::vector<long>> & _long_di;
  const std::vector<std::vector<SubdomainID>> & _subid_di;
  const std::vector<std::vector<BoundaryID>> & _bid_di;

  const std::vector<std::vector<std::string>> & _str_di;
  const std::vector<std::vector<FileName>> & _file_di;
  const std::vector<std::vector<FileNameNoExtension>> & _file_no_di;

  const std::vector<std::vector<MeshFileName>> & _mesh_file_di;
  const std::vector<std::vector<SubdomainName>> & _subdomain_name_di;
  const std::vector<std::vector<BoundaryName>> & _boundary_name_di;
  const std::vector<std::vector<FunctionName>> & _function_name_di;
  const std::vector<std::vector<UserObjectName>> & _userobject_name_di;
  const std::vector<std::vector<IndicatorName>> & _indicator_name_di;

  const std::vector<std::vector<MarkerName>> & _marker_name_di;
  const std::vector<std::vector<MultiAppName>> & _multiapp_name_di;
  const std::vector<std::vector<PostprocessorName>> & _postprocessor_name_di;
  const std::vector<std::vector<VectorPostprocessorName>> & _vector_postprocessor_name_di;
  const std::vector<std::vector<OutputName>> & _output_name_di;
  const std::vector<std::vector<MaterialPropertyName>> & _material_property_name_di;
};
