//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReadDoubleIndex.h"
#include <sstream>

registerMooseObject("MooseTestApp", ReadDoubleIndex);

InputParameters
ReadDoubleIndex::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::vector<std::vector<Real>>>("real_di",
                                                          "A double index field of real numbers.");
  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "uint_di", "A double index field of unsigned integers.");
  params.addRequiredParam<std::vector<std::vector<int>>>("int_di",
                                                         "A double index field of integers.");
  params.addRequiredParam<std::vector<std::vector<long>>>("long_di",
                                                          "A double index field of long integers.");
  params.addRequiredParam<std::vector<std::vector<SubdomainID>>>(
      "subid_di", "A double index field of SubdomainID.");
  params.addRequiredParam<std::vector<std::vector<BoundaryID>>>(
      "bid_di", "A double index field of SubdomainID.");
  params.addRequiredParam<std::vector<std::vector<std::string>>>(
      "str_di", "A double index field of std::string.");
  params.addRequiredParam<std::vector<std::vector<FileName>>>("file_di",
                                                              "A double index field of FileName.");
  params.addRequiredParam<std::vector<std::vector<FileNameNoExtension>>>(
      "file_no_di", "A double index field of FileNameNoExtension.");
  params.addRequiredParam<std::vector<std::vector<MeshFileName>>>(
      "mesh_file_di", "A double index field of MeshFileName.");
  params.addRequiredParam<std::vector<std::vector<SubdomainName>>>(
      "subdomain_name_di", "A double index field of SubdomainName.");
  params.addRequiredParam<std::vector<std::vector<BoundaryName>>>(
      "boundary_name_di", "A double index field of BoundaryName.");
  params.addRequiredParam<std::vector<std::vector<FunctionName>>>(
      "function_name_di", "A double index field of FunctionName.");
  params.addRequiredParam<std::vector<std::vector<UserObjectName>>>(
      "userobject_name_di", "A double index field of UserObjectName.");
  params.addRequiredParam<std::vector<std::vector<IndicatorName>>>(
      "indicator_name_di", "A double index field of IndicatorName.");
  params.addRequiredParam<std::vector<std::vector<MarkerName>>>(
      "marker_name_di", "A double index field of MarkerName.");
  params.addRequiredParam<std::vector<std::vector<MultiAppName>>>(
      "multiapp_name_di", "A double index field of MultiAppName.");
  params.addRequiredParam<std::vector<std::vector<PostprocessorName>>>(
      "postprocessor_name_di", "A double index field of PostprocessorName.");
  params.addRequiredParam<std::vector<std::vector<VectorPostprocessorName>>>(
      "vector_postprocessor_name_di", "A double index field of VectorPostprocessorName.");
  params.addRequiredParam<std::vector<std::vector<OutputName>>>(
      "output_name_di", "A double index field of OutputName.");
  params.addRequiredParam<std::vector<std::vector<MaterialPropertyName>>>(
      "material_property_name_di", "A double index field of MaterialPropertyName.");
  return params;
}

ReadDoubleIndex::ReadDoubleIndex(const InputParameters & params)
  : GeneralUserObject(params),
    _real_di(getParam<std::vector<std::vector<Real>>>("real_di")),
    _uint_di(getParam<std::vector<std::vector<unsigned int>>>("uint_di")),
    _int_di(getParam<std::vector<std::vector<int>>>("int_di")),
    _long_di(getParam<std::vector<std::vector<long>>>("long_di")),
    _subid_di(getParam<std::vector<std::vector<SubdomainID>>>("subid_di")),
    _bid_di(getParam<std::vector<std::vector<BoundaryID>>>("bid_di")),
    _str_di(getParam<std::vector<std::vector<std::string>>>("str_di")),
    _file_di(getParam<std::vector<std::vector<FileName>>>("file_di")),
    _file_no_di(getParam<std::vector<std::vector<FileNameNoExtension>>>("file_no_di")),
    _mesh_file_di(getParam<std::vector<std::vector<MeshFileName>>>("mesh_file_di")),
    _subdomain_name_di(getParam<std::vector<std::vector<SubdomainName>>>("subdomain_name_di")),
    _boundary_name_di(getParam<std::vector<std::vector<BoundaryName>>>("boundary_name_di")),
    _function_name_di(getParam<std::vector<std::vector<FunctionName>>>("function_name_di")),
    _userobject_name_di(getParam<std::vector<std::vector<UserObjectName>>>("userobject_name_di")),
    _indicator_name_di(getParam<std::vector<std::vector<IndicatorName>>>("indicator_name_di")),
    _marker_name_di(getParam<std::vector<std::vector<MarkerName>>>("marker_name_di")),
    _multiapp_name_di(getParam<std::vector<std::vector<MultiAppName>>>("multiapp_name_di")),
    _postprocessor_name_di(
        getParam<std::vector<std::vector<PostprocessorName>>>("postprocessor_name_di")),
    _vector_postprocessor_name_di(getParam<std::vector<std::vector<VectorPostprocessorName>>>(
        "vector_postprocessor_name_di")),
    _output_name_di(getParam<std::vector<std::vector<OutputName>>>("output_name_di")),
    _material_property_name_di(
        getParam<std::vector<std::vector<MaterialPropertyName>>>("material_property_name_di"))
{
  std::vector<unsigned int> array_length;
  array_length.resize(3);
  array_length[0] = 1;
  array_length[1] = 3;
  array_length[2] = 2;

  // check real
  if (_real_di.size() != 3)
    mooseError("Error reading real_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_real_di[j].size() != array_length[j])
      mooseError("Error reading real_di.");
    for (unsigned int i = 0; i < _real_di[j].size(); ++i)
      if (!MooseUtils::absoluteFuzzyEqual(_real_di[j][i], (j + 1.0) + 0.1 * (i + 1)))
        mooseError("Error reading real_di.");
  }

  // check unsigned int
  if (_uint_di.size() != 3)
    mooseError("Error reading uint_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_uint_di[j].size() != array_length[j])
      mooseError("Error reading uint_di.");
    for (unsigned int i = 0; i < _uint_di[j].size(); ++i)
      if (_uint_di[j][i] != (j + 1) * 10 + i + 1)
        mooseError("Error reading uint_di.");
  }

  // check int
  if (_int_di.size() != 3)
    mooseError("Error reading int_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_int_di[j].size() != array_length[j])
      mooseError("Error reading int_di.");
    for (unsigned int i = 0; i < _int_di[j].size(); ++i)
    {
      int mult = ((j == 1) ? -1 : 1);
      if (_int_di[j][i] != mult * static_cast<int>(((j + 1) * 10 + i + 1)))
        mooseError("Error reading int_di.");
    }
  }

  // check long
  if (_long_di.size() != 3)
    mooseError("Error reading long_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_long_di[j].size() != array_length[j])
      mooseError("Error reading long_di.");
    for (unsigned int i = 0; i < _long_di[j].size(); ++i)
    {
      long mult = ((j != 1) ? -1 : 1);
      if (_long_di[j][i] != mult * ((j + 1) * 10 + i + 1))
        mooseError("Error reading long_di.");
    }
  }

  // check SubdomainID
  if (_subid_di.size() != 3)
    mooseError("Error reading subid_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_subid_di[j].size() != array_length[j])
      mooseError("Error reading subid_di.");
    for (unsigned int i = 0; i < _subid_di[j].size(); ++i)
      if (_subid_di[j][i] != static_cast<SubdomainID>((j + 2) * 10 + i + 2))
        mooseError("Error reading subid_di.");
  }

  // check BoundaryID
  if (_bid_di.size() != 3)
    mooseError("Error reading bid_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_bid_di[j].size() != array_length[j])
      mooseError("Error reading bid_di.");
    for (unsigned int i = 0; i < _bid_di[j].size(); ++i)
      if (_bid_di[j][i] != static_cast<short>((j + 2) * 10 + i + 1))
        mooseError("Error reading bid_di.");
  }

  // check std::string
  if (_str_di.size() != 3)
    mooseError("Error reading str_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_str_di[j].size() != array_length[j])
      mooseError("Error reading str_di.");
    for (unsigned int i = 0; i < _str_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "string" << j << i;
      if (_str_di[j][i] != ss.str())
        mooseError("Error reading str_di.");
    }
  }

  // check FileName
  if (_file_di.size() != 3)
    mooseError("Error reading file_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_file_di[j].size() != array_length[j])
      mooseError("Error reading file_di.");
    for (unsigned int i = 0; i < _file_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "file" << j << i;
      if (_file_di[j][i] != ss.str())
        mooseError("Error reading file_di.");
    }
  }

  // check FileNameNoExtension
  if (_file_no_di.size() != 3)
    mooseError("Error reading file_no_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_file_no_di[j].size() != array_length[j])
      mooseError("Error reading file_no_di.");
    for (unsigned int i = 0; i < _file_no_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "file_no" << j << i;
      if (_file_no_di[j][i] != ss.str())
        mooseError("Error reading file_no_di.");
    }
  }

  // check MeshFileName
  if (_mesh_file_di.size() != 3)
    mooseError("Error reading mesh_file_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_mesh_file_di[j].size() != array_length[j])
      mooseError("Error reading mesh_file_di.");
    for (unsigned int i = 0; i < _mesh_file_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "mesh_file" << j << i;
      if (_mesh_file_di[j][i] != ss.str())
        mooseError("Error reading mesh_file_di.");
    }
  }

  // check SubdomainName
  if (_subdomain_name_di.size() != 3)
    mooseError("Error reading subdomain_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_subdomain_name_di[j].size() != array_length[j])
      mooseError("Error reading subdomain_name_di.");
    for (unsigned int i = 0; i < _subdomain_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "subdomain_name" << j << i;
      if (_subdomain_name_di[j][i] != ss.str())
        mooseError("Error reading subdomain_name_di.");
    }
  }

  // check BoundaryName
  if (_boundary_name_di.size() != 3)
    mooseError("Error reading boundary_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_boundary_name_di[j].size() != array_length[j])
      mooseError("Error reading boundary_name_di.");
    for (unsigned int i = 0; i < _boundary_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "boundary_name" << j << i;
      if (_boundary_name_di[j][i] != ss.str())
        mooseError("Error reading boundary_name_di.");
    }
  }

  // check FunctionName
  if (_function_name_di.size() != 3)
    mooseError("Error reading function_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_function_name_di[j].size() != array_length[j])
      mooseError("Error reading function_name_di.");
    for (unsigned int i = 0; i < _function_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "function_name" << j << i;
      if (_function_name_di[j][i] != ss.str())
        mooseError("Error reading function_name_di.");
    }
  }

  // check UserObjectName
  if (_userobject_name_di.size() != 3)
    mooseError("Error reading userobject_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_userobject_name_di[j].size() != array_length[j])
      mooseError("Error reading userobject_name_di.");
    for (unsigned int i = 0; i < _userobject_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "userobject_name" << j << i;
      if (_userobject_name_di[j][i] != ss.str())
        mooseError("Error reading userobject_name_di.");
    }
  }

  // check IndicatorName
  if (_indicator_name_di.size() != 3)
    mooseError("Error reading indicator_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_indicator_name_di[j].size() != array_length[j])
      mooseError("Error reading indicator_name_di.");
    for (unsigned int i = 0; i < _indicator_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "indicator_name" << j << i;
      if (_indicator_name_di[j][i] != ss.str())
        mooseError("Error reading indicator_name_di.");
    }
  }

  // check MarkerName
  if (_marker_name_di.size() != 3)
    mooseError("Error reading marker_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_marker_name_di[j].size() != array_length[j])
      mooseError("Error reading marker_name_di.");
    for (unsigned int i = 0; i < _marker_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "marker_name" << j << i;
      if (_marker_name_di[j][i] != ss.str())
        mooseError("Error reading marker_name_di.");
    }
  }

  // check MultiAppName
  if (_multiapp_name_di.size() != 3)
    mooseError("Error reading multiapp_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_multiapp_name_di[j].size() != array_length[j])
      mooseError("Error reading multiapp_name_di.");
    for (unsigned int i = 0; i < _multiapp_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "multiapp_name" << j << i;
      if (_multiapp_name_di[j][i] != ss.str())
        mooseError("Error reading multiapp_name_di.");
    }
  }

  // check PostprocessorName
  if (_postprocessor_name_di.size() != 3)
    mooseError("Error reading postprocessor_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_postprocessor_name_di[j].size() != array_length[j])
      mooseError("Error reading postprocessor_name_di.");
    for (unsigned int i = 0; i < _postprocessor_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "postprocessor_name" << j << i;
      if (_postprocessor_name_di[j][i] != ss.str())
        mooseError("Error reading postprocessor_name_di.");
    }
  }

  // check VectorPostprocessorName
  if (_vector_postprocessor_name_di.size() != 3)
    mooseError("Error reading vector_postprocessor_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_vector_postprocessor_name_di[j].size() != array_length[j])
      mooseError("Error reading vector_postprocessor_name_di.");
    for (unsigned int i = 0; i < _vector_postprocessor_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "vector_postprocessor_name" << j << i;
      if (_vector_postprocessor_name_di[j][i] != ss.str())
        mooseError("Error reading vector_postprocessor_name_di.");
    }
  }

  // check OutputName
  if (_output_name_di.size() != 3)
    mooseError("Error reading output_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_output_name_di[j].size() != array_length[j])
      mooseError("Error reading output_name_di.");
    for (unsigned int i = 0; i < _output_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "output_name" << j << i;
      if (_output_name_di[j][i] != ss.str())
        mooseError("Error reading output_name_di.");
    }
  }

  // check MaterialPropertyName
  if (_material_property_name_di.size() != 3)
    mooseError("Error reading material_property_name_di.");
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (_material_property_name_di[j].size() != array_length[j])
      mooseError("Error reading material_property_name_di.");
    for (unsigned int i = 0; i < _material_property_name_di[j].size(); ++i)
    {
      std::stringstream ss;
      ss << "material_property_name" << j << i;
      if (_material_property_name_di[j][i] != ss.str())
        mooseError("Error reading material_property_name_di.");
    }
  }
}

void
ReadDoubleIndex::execute()
{
}
