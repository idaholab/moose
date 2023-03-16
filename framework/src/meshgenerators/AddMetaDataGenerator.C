//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMetaDataGenerator.h"

#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", AddMetaDataGenerator);

InputParameters
AddMetaDataGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "This mesh generator assigns extraneous mesh metadata to the input mesh");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  params.addParam<std::vector<std::string>>("real_scalar_metadata_names",
                                            "Names of the real scalar mesh metadata.");
  params.addParam<std::vector<Real>>("real_scalar_metadata_values",
                                     "Values of the real scalar mesh metadata.");
  params.addParam<std::vector<std::string>>("uint_scalar_metadata_names",
                                            "Names of the unsigned integer scalar mesh metadata.");
  params.addParam<std::vector<unsigned int>>(
      "uint_scalar_metadata_values", "Values of the unsigned integer scalar mesh metadata.");
  params.addParam<std::vector<std::string>>("int_scalar_metadata_names",
                                            "Names of the integer scalar mesh metadata.");
  params.addParam<std::vector<int>>("int_scalar_metadata_values",
                                    "Values of the integer scalar mesh metadata.");
  params.addParam<std::vector<std::string>>("dof_id_type_scalar_metadata_names",
                                            "Names of the dof_id_type scalar mesh metadata.");
  params.addParam<std::vector<dof_id_type>>("dof_id_type_scalar_metadata_values",
                                            "Values of the dof_id_type scalar mesh metadata.");
  params.addParam<std::vector<std::string>>("subdomain_id_type_scalar_metadata_names",
                                            "Names of the subdomain_id_type scalar mesh metadata.");
  params.addParam<std::vector<subdomain_id_type>>(
      "subdomain_id_type_scalar_metadata_values",
      "Values of the subdomain_id_type scalar mesh metadata.");
  params.addParam<std::vector<std::string>>("boolean_scalar_metadata_names",
                                            "Names of the boolean scalar mesh metadata.");
  params.addParam<std::vector<bool>>("boolean_scalar_metadata_values",
                                     "Values of the boolean scalar mesh metadata.");
  params.addParam<std::vector<std::string>>("point_scalar_metadata_names",
                                            "Names of the point scalar mesh metadata.");
  params.addParam<std::vector<Point>>("point_scalar_metadata_values",
                                      "Values of the point scalar mesh metadata.");

  params.addParam<std::vector<std::string>>("real_vector_metadata_names",
                                            "Names of the real vector mesh metadata.");
  params.addParam<std::vector<std::vector<Real>>>("real_vector_metadata_values",
                                                  "Values of the real vector mesh metadata.");
  params.addParam<std::vector<std::string>>("uint_vector_metadata_names",
                                            "Names of the unsigned integer vector mesh metadata.");
  params.addParam<std::vector<std::vector<unsigned int>>>(
      "uint_vector_metadata_values", "Values of the unsigned integer vector mesh metadata.");
  params.addParam<std::vector<std::string>>("int_vector_metadata_names",
                                            "Names of the integer vector mesh metadata.");
  params.addParam<std::vector<std::vector<int>>>("int_vector_metadata_values",
                                                 "Values of the integer vector mesh metadata.");
  params.addParam<std::vector<std::string>>("dof_id_type_vector_metadata_names",
                                            "Names of the dof_id_type vector mesh metadata.");
  params.addParam<std::vector<std::vector<dof_id_type>>>(
      "dof_id_type_vector_metadata_values", "Values of the dof_id_type vector mesh metadata.");
  params.addParam<std::vector<std::string>>("subdomain_id_type_vector_metadata_names",
                                            "Names of the subdomain_id_type vector mesh metadata.");
  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "subdomain_id_type_vector_metadata_values",
      "Values of the subdomain_id_type vector mesh metadata.");
  params.addParam<std::vector<std::string>>("point_vector_metadata_names",
                                            "Names of the Point vector mesh metadata.");
  params.addParam<std::vector<std::vector<Point>>>("point_vector_metadata_values",
                                                   "Values of the Point vector mesh metadata.");

  return params;
}

AddMetaDataGenerator::AddMetaDataGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _real_scalar_metadata_names(getParam<std::vector<std::string>>("real_scalar_metadata_names")),
    _real_scalar_metadata_values(getParam<std::vector<Real>>("real_scalar_metadata_values")),
    _uint_scalar_metadata_names(getParam<std::vector<std::string>>("uint_scalar_metadata_names")),
    _uint_scalar_metadata_values(
        getParam<std::vector<unsigned int>>("uint_scalar_metadata_values")),
    _int_scalar_metadata_names(getParam<std::vector<std::string>>("int_scalar_metadata_names")),
    _int_scalar_metadata_values(getParam<std::vector<int>>("int_scalar_metadata_values")),
    _dof_id_type_scalar_metadata_names(
        getParam<std::vector<std::string>>("dof_id_type_scalar_metadata_names")),
    _dof_id_type_scalar_metadata_values(
        getParam<std::vector<dof_id_type>>("dof_id_type_scalar_metadata_values")),
    _subdomain_id_type_scalar_metadata_names(
        getParam<std::vector<std::string>>("subdomain_id_type_scalar_metadata_names")),
    _subdomain_id_type_scalar_metadata_values(
        getParam<std::vector<subdomain_id_type>>("subdomain_id_type_scalar_metadata_values")),
    _boolean_scalar_metadata_names(
        getParam<std::vector<std::string>>("boolean_scalar_metadata_names")),
    _boolean_scalar_metadata_values(getParam<std::vector<bool>>("boolean_scalar_metadata_values")),
    _point_scalar_metadata_names(getParam<std::vector<std::string>>("point_scalar_metadata_names")),
    _point_scalar_metadata_values(getParam<std::vector<Point>>("point_scalar_metadata_values")),
    _real_vector_metadata_names(getParam<std::vector<std::string>>("real_vector_metadata_names")),
    _real_vector_metadata_values(
        getParam<std::vector<std::vector<Real>>>("real_vector_metadata_values")),
    _uint_vector_metadata_names(getParam<std::vector<std::string>>("uint_vector_metadata_names")),
    _uint_vector_metadata_values(
        getParam<std::vector<std::vector<unsigned int>>>("uint_vector_metadata_values")),
    _int_vector_metadata_names(getParam<std::vector<std::string>>("int_vector_metadata_names")),
    _int_vector_metadata_values(
        getParam<std::vector<std::vector<int>>>("int_vector_metadata_values")),
    _dof_id_type_vector_metadata_names(
        getParam<std::vector<std::string>>("dof_id_type_vector_metadata_names")),
    _dof_id_type_vector_metadata_values(
        getParam<std::vector<std::vector<dof_id_type>>>("dof_id_type_vector_metadata_values")),
    _subdomain_id_type_vector_metadata_names(
        getParam<std::vector<std::string>>("subdomain_id_type_vector_metadata_names")),
    _subdomain_id_type_vector_metadata_values(getParam<std::vector<std::vector<subdomain_id_type>>>(
        "subdomain_id_type_vector_metadata_values")),
    _point_vector_metadata_names(getParam<std::vector<std::string>>("point_vector_metadata_names")),
    _point_vector_metadata_values(
        getParam<std::vector<std::vector<Point>>>("point_vector_metadata_values"))
{
  inputChecker(_real_scalar_metadata_names, _real_scalar_metadata_values, "real_scalar");
  for (unsigned int i = 0; i < _real_scalar_metadata_names.size(); i++)
    declareMeshProperty<Real>(_real_scalar_metadata_names[i], _real_scalar_metadata_values[i]);

  inputChecker(_uint_scalar_metadata_names, _uint_scalar_metadata_values, "uint_scalar");
  for (unsigned int i = 0; i < _uint_scalar_metadata_names.size(); i++)
    declareMeshProperty<unsigned int>(_uint_scalar_metadata_names[i],
                                      _uint_scalar_metadata_values[i]);

  inputChecker(_int_scalar_metadata_names, _int_scalar_metadata_values, "int_scalar");
  for (unsigned int i = 0; i < _int_scalar_metadata_names.size(); i++)
    declareMeshProperty<int>(_int_scalar_metadata_names[i], _int_scalar_metadata_values[i]);

  inputChecker(_dof_id_type_scalar_metadata_names,
               _dof_id_type_scalar_metadata_values,
               "dof_id_type_scalar");
  for (unsigned int i = 0; i < _dof_id_type_scalar_metadata_names.size(); i++)
    declareMeshProperty<dof_id_type>(_dof_id_type_scalar_metadata_names[i],
                                     _dof_id_type_scalar_metadata_values[i]);

  inputChecker(_subdomain_id_type_scalar_metadata_names,
               _subdomain_id_type_scalar_metadata_values,
               "subdomain_id_type_scalar");
  for (unsigned int i = 0; i < _subdomain_id_type_scalar_metadata_names.size(); i++)
    declareMeshProperty<subdomain_id_type>(_subdomain_id_type_scalar_metadata_names[i],
                                           _subdomain_id_type_scalar_metadata_values[i]);

  inputChecker(_boolean_scalar_metadata_names, _boolean_scalar_metadata_values, "boolean_scalar");
  for (unsigned int i = 0; i < _boolean_scalar_metadata_names.size(); i++)
    declareMeshProperty<bool>(_boolean_scalar_metadata_names[i],
                              _boolean_scalar_metadata_values[i]);

  inputChecker(_point_scalar_metadata_names, _point_scalar_metadata_values, "point_scalar");
  for (unsigned int i = 0; i < _point_scalar_metadata_names.size(); i++)
    declareMeshProperty<Point>(_point_scalar_metadata_names[i], _point_scalar_metadata_values[i]);

  inputChecker(_real_vector_metadata_names, _real_vector_metadata_values, "real_vector");
  for (unsigned int i = 0; i < _real_vector_metadata_names.size(); i++)
    declareMeshProperty<std::vector<Real>>(_real_vector_metadata_names[i],
                                           _real_vector_metadata_values[i]);

  inputChecker(_uint_vector_metadata_names, _uint_vector_metadata_values, "uint_vector");
  for (unsigned int i = 0; i < _uint_vector_metadata_names.size(); i++)
    declareMeshProperty<std::vector<unsigned int>>(_uint_vector_metadata_names[i],
                                                   _uint_vector_metadata_values[i]);

  inputChecker(_int_vector_metadata_names, _int_vector_metadata_values, "int_vector");
  for (unsigned int i = 0; i < _int_vector_metadata_names.size(); i++)
    declareMeshProperty<std::vector<int>>(_int_vector_metadata_names[i],
                                          _int_vector_metadata_values[i]);

  inputChecker(_dof_id_type_vector_metadata_names,
               _dof_id_type_vector_metadata_values,
               "dof_id_type_vector");
  for (unsigned int i = 0; i < _dof_id_type_vector_metadata_names.size(); i++)
    declareMeshProperty<std::vector<dof_id_type>>(_dof_id_type_vector_metadata_names[i],
                                                  _dof_id_type_vector_metadata_values[i]);

  inputChecker(_subdomain_id_type_vector_metadata_names,
               _subdomain_id_type_vector_metadata_values,
               "subdomain_id_type_vector");
  for (unsigned int i = 0; i < _subdomain_id_type_vector_metadata_names.size(); i++)
    declareMeshProperty<std::vector<subdomain_id_type>>(
        _subdomain_id_type_vector_metadata_names[i], _subdomain_id_type_vector_metadata_values[i]);

  inputChecker(_point_vector_metadata_names, _point_vector_metadata_values, "point_vector");
  for (unsigned int i = 0; i < _point_vector_metadata_names.size(); i++)
    declareMeshProperty<std::vector<Point>>(_point_vector_metadata_names[i],
                                            _point_vector_metadata_values[i]);
}

std::unique_ptr<MeshBase>
AddMetaDataGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  return dynamic_pointer_cast<MeshBase>(mesh);
}

template <class T>
void
AddMetaDataGenerator::inputChecker(const std::vector<std::string> data_names,
                                   const std::vector<T> data_values,
                                   const std::string param_name)
{
  std::vector<std::string> data_name_tmp(data_names);
  if (std::unique(data_name_tmp.begin(), data_name_tmp.end()) != data_name_tmp.end())
    paramError(param_name + "_metadata_names", "Elements of this parameter must be unique.");
  if (data_names.size() != data_values.size())
    paramError(param_name + "_metadata_values",
               "Length of this parameter must be the same as that of " + param_name +
                   "_metadata_names");
}
