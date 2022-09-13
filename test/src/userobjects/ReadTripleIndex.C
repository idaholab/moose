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
      "real_tri", "A double index field of real numbers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<unsigned int>>>>(
      "uint_tri", "A double index field of unsigned integers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<int>>>>(
      "int_tri", "A double index field of integers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<long>>>>(
      "long_tri", "A double index field of long integers.");
  params.addRequiredParam<std::vector<std::vector<std::vector<SubdomainID>>>>(
      "subid_tri", "A double index field of SubdomainID.");
  params.addRequiredParam<std::vector<std::vector<std::vector<BoundaryID>>>>(
      "bid_tri", "A double index field of SubdomainID.");
  params.addRequiredParam<std::vector<std::vector<std::vector<std::string>>>>(
      "str_tri", "A double index field of std::string.");
  params.addRequiredParam<std::vector<std::vector<std::vector<FileName>>>>(
      "file_tri", "A double index field of FileName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<FileNameNoExtension>>>>(
      "file_no_tri", "A double index field of FileNameNoExtension.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MeshFileName>>>>(
      "mesh_file_tri", "A double index field of MeshFileName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<SubdomainName>>>>(
      "subdomain_name_tri", "A double index field of SubdomainName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<BoundaryName>>>>(
      "boundary_name_tri", "A double index field of BoundaryName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<FunctionName>>>>(
      "function_name_tri", "A double index field of FunctionName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<UserObjectName>>>>(
      "userobject_name_tri", "A double index field of UserObjectName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<IndicatorName>>>>(
      "indicator_name_tri", "A double index field of IndicatorName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MarkerName>>>>(
      "marker_name_tri", "A double index field of MarkerName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MultiAppName>>>>(
      "multiapp_name_tri", "A double index field of MultiAppName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<PostprocessorName>>>>(
      "postprocessor_name_tri", "A double index field of PostprocessorName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<VectorPostprocessorName>>>>(
      "vector_postprocessor_name_tri", "A double index field of VectorPostprocessorName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<OutputName>>>>(
      "output_name_tri", "A double index field of OutputName.");
  params.addRequiredParam<std::vector<std::vector<std::vector<MaterialPropertyName>>>>(
      "material_property_name_tri", "A double index field of MaterialPropertyName.");
  return params;
}

ReadTripleIndex::ReadTripleIndex(const InputParameters & params)
  : GeneralUserObject(params),
    _real_tri(getParam<std::vector<std::vector<std::vector<Real>>>>("real_tri")),
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
            "material_property_name_tri"))
{
  const std::vector<std::vector<std::vector<Real>>> reference_values = {
      {{1.1}, {2.1, 2.2, 2.3}, {3.1, 3.2}},
      {{11.1, 11.2}},
      {{21.1, 21.2}, {22.1}, {23.1, 23.2, 23.3}}};

  // check real
  if (_real_tri != reference_values)
    mooseError("Error reading real_tri.");

  // check unsigned int
  if (_uint_tri.size() != reference_values.size())
    mooseError("Error reading uint_tri.");
  for (unsigned int i = 0; i < _uint_tri.size(); i++)
  {
    if (_uint_tri[i].size() != reference_values[i].size())
      mooseError("Error reading uint_tri.");
    for (unsigned int j = 0; j < _uint_tri[i].size(); j++)
    {
      if (_uint_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading uint_tri.");
      for (unsigned int k = 0; k < _uint_tri[i][j].size(); k++)
        if (_uint_tri[i][j][k] != static_cast<unsigned int>(reference_values[i][j][k] * 10.0))
          mooseError("Error reading uint_tri.");
    }
  }

  // check int
  if (_int_tri.size() != reference_values.size())
    mooseError("Error reading int_tri.");
  for (unsigned int i = 0; i < _int_tri.size(); i++)
  {
    if (_int_tri[i].size() != reference_values[i].size())
      mooseError("Error reading int_tri.");
    for (unsigned int j = 0; j < _int_tri[i].size(); j++)
    {
      if (_int_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading int_tri.");
      for (unsigned int k = 0; k < _int_tri[i][j].size(); k++)
        if (_int_tri[i][j][k] != (i % 2 ? 1 : -1) * (j % 2 ? 1 : -1) *
                                     static_cast<int>(reference_values[i][j][k] * 10.0))
          mooseError("Error reading int_tri.");
    }
  }

  // check long
  if (_long_tri.size() != reference_values.size())
    mooseError("Error reading long_tri.");
  for (unsigned int i = 0; i < _long_tri.size(); i++)
  {
    if (_long_tri[i].size() != reference_values[i].size())
      mooseError("Error reading long_tri.");
    for (unsigned int j = 0; j < _long_tri[i].size(); j++)
    {
      if (_long_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading long_tri.");
      for (unsigned int k = 0; k < _long_tri[i][j].size(); k++)
        if (_long_tri[i][j][k] != (i % 2 ? 1 : -1) * (j % 2 ? -1 : 1) *
                                      static_cast<long>(reference_values[i][j][k] * 10.0))
          mooseError("Error reading long_tri.");
    }
  }

  // check SubdomainID
  if (_subid_tri.size() != reference_values.size())
    mooseError("Error reading subid_tri.");
  for (unsigned int i = 0; i < _subid_tri.size(); i++)
  {
    if (_subid_tri[i].size() != reference_values[i].size())
      mooseError("Error reading subid_tri.");
    for (unsigned int j = 0; j < _subid_tri[i].size(); j++)
    {
      if (_subid_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading subid_tri.");
      for (unsigned int k = 0; k < _subid_tri[i][j].size(); k++)
        if (_subid_tri[i][j][k] !=
            static_cast<SubdomainID>(reference_values[i][j][k] * 10.0 + 30.0))
          mooseError("Error reading subid_tri.");
    }
  }

  // check BoundaryID
  if (_bid_tri.size() != reference_values.size())
    mooseError("Error reading bid_tri.");
  for (unsigned int i = 0; i < _bid_tri.size(); i++)
  {
    if (_bid_tri[i].size() != reference_values[i].size())
      mooseError("Error reading bid_tri.");
    for (unsigned int j = 0; j < _bid_tri[i].size(); j++)
    {
      if (_bid_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading bid_tri.");
      for (unsigned int k = 0; k < _bid_tri[i][j].size(); k++)
        if (_bid_tri[i][j][k] != static_cast<BoundaryID>(reference_values[i][j][k] * 10.0 + 35.0))
          mooseError("Error reading bid_tri.");
    }
  }

  // check std::string
  if (_str_tri.size() != reference_values.size())
    mooseError("Error reading str_tri.");
  for (unsigned int i = 0; i < _str_tri.size(); i++)
  {
    if (_str_tri[i].size() != reference_values[i].size())
      mooseError("Error reading str_tri.");
    for (unsigned int j = 0; j < _str_tri[i].size(); j++)
    {
      if (_str_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading str_tri.");
      for (unsigned int k = 0; k < _str_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "string" << i << j << k;
        if (_str_tri[i][j][k] != ss.str())
          mooseError("Error reading str_tri.");
      }
    }
  }

  // check FileName
  if (_file_tri.size() != reference_values.size())
    mooseError("Error reading file_tri.");
  for (unsigned int i = 0; i < _file_tri.size(); i++)
  {
    if (_file_tri[i].size() != reference_values[i].size())
      mooseError("Error reading file_tri.");
    for (unsigned int j = 0; j < _file_tri[i].size(); j++)
    {
      if (_file_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading file_tri.");
      for (unsigned int k = 0; k < _file_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "file" << i << j << k;
        if (_file_tri[i][j][k] != ss.str())
          mooseError("Error reading file_tri.");
      }
    }
  }

  // check FileNameNoExtension
  if (_file_no_tri.size() != reference_values.size())
    mooseError("Error reading file_no_tri.");
  for (unsigned int i = 0; i < _file_no_tri.size(); i++)
  {
    if (_file_no_tri[i].size() != reference_values[i].size())
      mooseError("Error reading file_no_tri.");
    for (unsigned int j = 0; j < _file_no_tri[i].size(); j++)
    {
      if (_file_no_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading file_no_tri.");
      for (unsigned int k = 0; k < _file_no_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "file_no" << i << j << k;
        if (_file_no_tri[i][j][k] != ss.str())
          mooseError("Error reading file_no_tri.");
      }
    }
  }

  // check MeshFileName
  if (_mesh_file_tri.size() != reference_values.size())
    mooseError("Error reading mesh_file_tri.");
  for (unsigned int i = 0; i < _mesh_file_tri.size(); i++)
  {
    if (_mesh_file_tri[i].size() != reference_values[i].size())
      mooseError("Error reading mesh_file_tri.");
    for (unsigned int j = 0; j < _mesh_file_tri[i].size(); j++)
    {
      if (_mesh_file_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading mesh_file_tri.");
      for (unsigned int k = 0; k < _mesh_file_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "mesh_file" << i << j << k;
        if (_mesh_file_tri[i][j][k] != ss.str())
          mooseError("Error reading mesh_file_tri.");
      }
    }
  }

  // check SubdomainName
  if (_subdomain_name_tri.size() != reference_values.size())
    mooseError("Error reading subdomain_name_tri.");
  for (unsigned int i = 0; i < _subdomain_name_tri.size(); i++)
  {
    if (_subdomain_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading subdomain_name_tri.");
    for (unsigned int j = 0; j < _subdomain_name_tri[i].size(); j++)
    {
      if (_subdomain_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading subdomain_name_tri.");
      for (unsigned int k = 0; k < _subdomain_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "subdomain_name" << i << j << k;
        if (_subdomain_name_tri[i][j][k] != ss.str())
          mooseError("Error reading subdomain_name_tri.");
      }
    }
  }

  // check BoundaryName
  if (_boundary_name_tri.size() != reference_values.size())
    mooseError("Error reading boundary_name_tri.");
  for (unsigned int i = 0; i < _boundary_name_tri.size(); i++)
  {
    if (_boundary_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading boundary_name_tri.");
    for (unsigned int j = 0; j < _boundary_name_tri[i].size(); j++)
    {
      if (_boundary_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading boundary_name_tri.");
      for (unsigned int k = 0; k < _boundary_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "boundary_name" << i << j << k;
        if (_boundary_name_tri[i][j][k] != ss.str())
          mooseError("Error reading boundary_name_tri.");
      }
    }
  }

  // check FunctionName
  if (_function_name_tri.size() != reference_values.size())
    mooseError("Error reading function_name_tri.");
  for (unsigned int i = 0; i < _function_name_tri.size(); i++)
  {
    if (_function_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading function_name_tri.");
    for (unsigned int j = 0; j < _function_name_tri[i].size(); j++)
    {
      if (_function_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading function_name_tri.");
      for (unsigned int k = 0; k < _function_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "function_name" << i << j << k;
        if (_function_name_tri[i][j][k] != ss.str())
          mooseError("Error reading function_name_tri.");
      }
    }
  }

  // check UserObjectName
  if (_userobject_name_tri.size() != reference_values.size())
    mooseError("Error reading userobject_name_tri.");
  for (unsigned int i = 0; i < _userobject_name_tri.size(); i++)
  {
    if (_userobject_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading userobject_name_tri.");
    for (unsigned int j = 0; j < _userobject_name_tri[i].size(); j++)
    {
      if (_userobject_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading userobject_name_tri.");
      for (unsigned int k = 0; k < _userobject_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "userobject_name" << i << j << k;
        if (_userobject_name_tri[i][j][k] != ss.str())
          mooseError("Error reading userobject_name_tri.");
      }
    }
  }

  // check IndicatorName
  if (_indicator_name_tri.size() != reference_values.size())
    mooseError("Error reading indicator_name_tri.");
  for (unsigned int i = 0; i < _indicator_name_tri.size(); i++)
  {
    if (_indicator_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading indicator_name_tri.");
    for (unsigned int j = 0; j < _indicator_name_tri[i].size(); j++)
    {
      if (_indicator_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading indicator_name_tri.");
      for (unsigned int k = 0; k < _indicator_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "indicator_name" << i << j << k;
        if (_indicator_name_tri[i][j][k] != ss.str())
          mooseError("Error reading indicator_name_tri.");
      }
    }
  }

  // check MarkerName
  if (_marker_name_tri.size() != reference_values.size())
    mooseError("Error reading marker_name_tri.");
  for (unsigned int i = 0; i < _marker_name_tri.size(); i++)
  {
    if (_marker_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading marker_name_tri.");
    for (unsigned int j = 0; j < _marker_name_tri[i].size(); j++)
    {
      if (_marker_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading marker_name_tri.");
      for (unsigned int k = 0; k < _marker_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "marker_name" << i << j << k;
        if (_marker_name_tri[i][j][k] != ss.str())
          mooseError("Error reading marker_name_tri.");
      }
    }
  }

  // check MultiAppName
  if (_multiapp_name_tri.size() != reference_values.size())
    mooseError("Error reading multiapp_name_tri.");
  for (unsigned int i = 0; i < _multiapp_name_tri.size(); i++)
  {
    if (_multiapp_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading multiapp_name_tri.");
    for (unsigned int j = 0; j < _multiapp_name_tri[i].size(); j++)
    {
      if (_multiapp_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading multiapp_name_tri.");
      for (unsigned int k = 0; k < _multiapp_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "multiapp_name" << i << j << k;
        if (_multiapp_name_tri[i][j][k] != ss.str())
          mooseError("Error reading multiapp_name_tri.");
      }
    }
  }

  // check PostprocessorName
  if (_postprocessor_name_tri.size() != reference_values.size())
    mooseError("Error reading postprocessor_name_tri.");
  for (unsigned int i = 0; i < _postprocessor_name_tri.size(); i++)
  {
    if (_postprocessor_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading postprocessor_name_tri.");
    for (unsigned int j = 0; j < _postprocessor_name_tri[i].size(); j++)
    {
      if (_postprocessor_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading postprocessor_name_tri.");
      for (unsigned int k = 0; k < _postprocessor_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "postprocessor_name" << i << j << k;
        if (_postprocessor_name_tri[i][j][k] != ss.str())
          mooseError("Error reading postprocessor_name_tri.");
      }
    }
  }

  // check VectorPostprocessorName
  if (_vector_postprocessor_name_tri.size() != reference_values.size())
    mooseError("Error reading vector_postprocessor_name_tri.");
  for (unsigned int i = 0; i < _vector_postprocessor_name_tri.size(); i++)
  {
    if (_vector_postprocessor_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading vector_postprocessor_name_tri.");
    for (unsigned int j = 0; j < _vector_postprocessor_name_tri[i].size(); j++)
    {
      if (_vector_postprocessor_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading vector_postprocessor_name_tri.");
      for (unsigned int k = 0; k < _vector_postprocessor_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "vector_postprocessor_name" << i << j << k;
        if (_vector_postprocessor_name_tri[i][j][k] != ss.str())
          mooseError("Error reading vector_postprocessor_name_tri.");
      }
    }
  }

  // check OutputName
  if (_output_name_tri.size() != reference_values.size())
    mooseError("Error reading output_name_tri.");
  for (unsigned int i = 0; i < _output_name_tri.size(); i++)
  {
    if (_output_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading output_name_tri.");
    for (unsigned int j = 0; j < _output_name_tri[i].size(); j++)
    {
      if (_output_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading output_name_tri.");
      for (unsigned int k = 0; k < _output_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "output_name" << i << j << k;
        if (_output_name_tri[i][j][k] != ss.str())
          mooseError("Error reading output_name_tri.");
      }
    }
  }

  // check MaterialPropertyName
  if (_material_property_name_tri.size() != reference_values.size())
    mooseError("Error reading _material_property_name_tri.");
  for (unsigned int i = 0; i < _material_property_name_tri.size(); i++)
  {
    if (_material_property_name_tri[i].size() != reference_values[i].size())
      mooseError("Error reading _material_property_name_tri.");
    for (unsigned int j = 0; j < _material_property_name_tri[i].size(); j++)
    {
      if (_material_property_name_tri[i][j].size() != reference_values[i][j].size())
        mooseError("Error reading _material_property_name_tri.");
      for (unsigned int k = 0; k < _material_property_name_tri[i][j].size(); k++)
      {
        std::stringstream ss;
        ss << "material_property_name" << i << j << k;
        if (_material_property_name_tri[i][j][k] != ss.str())
          mooseError("Error reading _material_property_name_tri.");
      }
    }
  }
}

void
ReadTripleIndex::execute()
{
}
