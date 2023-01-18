//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * This mesh generator assigns external mesh metadata to the input mesh
 */
class AddMetaDataGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  AddMetaDataGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// The input mesh to add the mesh metadata to
  std::unique_ptr<MeshBase> & _input;
  /// List of mesh metadata names for the Real type scalars
  const std::vector<std::string> _real_scalar_metadata_names;
  /// List of mesh metadata values for the Real type scalars
  const std::vector<Real> _real_scalar_metadata_values;
  /// List of mesh metadata names for the unsigned integer type scalars
  const std::vector<std::string> _uint_scalar_metadata_names;
  /// List of mesh metadata values for the unsigned integer type scalars
  const std::vector<unsigned int> _uint_scalar_metadata_values;
  /// List of mesh metadata names for the integer type scalars
  const std::vector<std::string> _int_scalar_metadata_names;
  /// List of mesh metadata values for the integer type scalars
  const std::vector<int> _int_scalar_metadata_values;
  /// List of mesh metadata names for the dof_id_type type scalars
  const std::vector<std::string> _dof_id_type_scalar_metadata_names;
  /// List of mesh metadata values for the dof_id_type type scalars
  const std::vector<dof_id_type> _dof_id_type_scalar_metadata_values;
  /// List of mesh metadata names for the subdomain_id_type type scalars
  const std::vector<std::string> _subdomain_id_type_scalar_metadata_names;
  /// List of mesh metadata values for the subdomain_id_type type scalars
  const std::vector<subdomain_id_type> _subdomain_id_type_scalar_metadata_values;
  /// List of mesh metadata names for the boolean type scalars
  const std::vector<std::string> _boolean_scalar_metadata_names;
  /// List of mesh metadata values for the boolean type scalars
  const std::vector<bool> _boolean_scalar_metadata_values;
  /// List of mesh metadata names for single Point metadata
  const std::vector<std::string> _point_scalar_metadata_names;
  /// List of mesh metadata values for single Point metadata
  const std::vector<Point> _point_scalar_metadata_values;

  /// List of mesh metadata names for the Real type vectors
  const std::vector<std::string> _real_vector_metadata_names;
  /// List of mesh metadata values for the Real type vectors
  const std::vector<std::vector<Real>> _real_vector_metadata_values;
  /// List of mesh metadata names for the unsigned integer type vectors
  const std::vector<std::string> _uint_vector_metadata_names;
  /// List of mesh metadata values for the unsigned integer type vectors
  const std::vector<std::vector<unsigned int>> _uint_vector_metadata_values;
  /// List of mesh metadata names for the integer type vectors
  const std::vector<std::string> _int_vector_metadata_names;
  /// List of mesh metadata values for the integer type vectors
  const std::vector<std::vector<int>> _int_vector_metadata_values;
  /// List of mesh metadata names for the dof_id_type type vectors
  const std::vector<std::string> _dof_id_type_vector_metadata_names;
  /// List of mesh metadata values for the dof_id_type type vectors
  const std::vector<std::vector<dof_id_type>> _dof_id_type_vector_metadata_values;
  /// List of mesh metadata names for the subdomain_id_type type vectors
  const std::vector<std::string> _subdomain_id_type_vector_metadata_names;
  /// List of mesh metadata values for the subdomain_id_type type vectors
  const std::vector<std::vector<subdomain_id_type>> _subdomain_id_type_vector_metadata_values;
  /// List of mesh metadata names for the Point type vectors
  const std::vector<std::string> _point_vector_metadata_names;
  /// List of mesh metadata values for the Point type vectors
  const std::vector<std::vector<Point>> _point_vector_metadata_values;

  /**
   * Check the sanity of a pair of input parameters
   * @param data_names the name component of the input parameter pair
   * @param data_values the value component of the input parameter pair
   * @param param_name the key description word of the input parameter pairs
   */
  template <class T>
  void inputChecker(const std::vector<std::string> data_names,
                    const std::vector<T> data_values,
                    const std::string param_name);
};
