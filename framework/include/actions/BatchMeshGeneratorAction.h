//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"
#include "ActionWarehouse.h"

/**
 * Action to generate batches of mesh generators from vectors of parameter values
 */
class BatchMeshGeneratorAction : public Action
{
public:
  static InputParameters validParams();

  BatchMeshGeneratorAction(const InputParameters & params);

  virtual void act() override final;

  enum class MultiBatchParamsMethod
  {
    corresponding,
    cartesian_product
  };

  enum class ParameterType
  {
    BOOL,
    REAL,
    SHORT,
    USHORT,
    INT,
    UINT,
    STRING,
    ENUM
  };

protected:
  virtual void addMeshGenerators();
  /// Name of the mesh generator to use for batch generation
  const std::string _mesh_generator_name;
  /// Prefix to use for naming the batch generated meshes
  const std::string _mesh_name_prefix;
  /// Names of the scalar input parameters to vary in the batch generation
  const std::vector<std::string> _batch_scalar_input_param_names;
  /// Types of the scalar input parameters to vary in the batch generation
  const std::vector<ParameterType> _batch_scalar_input_param_types;
  /// Values of the scalar input parameters to vary in the batch generation
  const std::vector<std::vector<std::string>> _batch_scalar_input_params;
  /// Names of the vector input parameters to vary in the batch generation
  const std::vector<std::string> _batch_vector_input_param_names;
  /// Types of the vector input parameters to vary in the batch generation
  const std::vector<ParameterType> _batch_vector_input_param_types;
  /// Values of the vector input parameters to vary in the batch generation
  const std::vector<std::vector<std::vector<std::string>>> _batch_vector_input_params;
  /// Method to use for generating the batch parameters
  const MultiBatchParamsMethod _multi_batch_params_method;
  /// Names of the vector input parameters to keep fixed in the batch generation
  const std::vector<std::string> _fixed_scalar_input_param_names;
  /// Types of the vector input parameters to keep fixed in the batch generation
  const std::vector<ParameterType> _fixed_scalar_input_param_types;
  /// Values of the vector input parameters to keep fixed in the batch generation
  const std::vector<std::string> _fixed_scalar_input_param_values;
  /// Names of the vector input parameters to keep fixed in the batch generation
  const std::vector<std::string> _fixed_vector_input_param_names;
  /// Types of the vector input parameters to keep fixed in the batch generation
  const std::vector<ParameterType> _fixed_vector_input_param_types;
  /// Values of the vector input parameters to keep fixed in the batch generation
  const std::vector<std::vector<std::string>> _fixed_vector_input_param_values;
  /// Flag to indicate if the decomposed index should be used in the mesh name
  const bool _use_decomposed_index;

  /**
   * Set the scalar input parameters for a unit mesh generator
   * @param params InputParameters object to set the parameters
   * @param param_name Name of the parameter to set
   * @param param_type Type of the parameter to set
   * @param param_value Value of the parameter to set in string format
   */
  void setScalarParams(InputParameters & params,
                       const std::string & param_name,
                       const ParameterType & param_type,
                       const std::string & param_value);

  /**
   * Set the vector input parameters for a unit mesh generator
   * @param params InputParameters object to set the parameters
   * @param param_name Name of the parameter to set
   * @param param_type Type of the parameter to set
   * @param param_value Value of the parameter to set in string format
   */
  void setVectorParams(InputParameters & params,
                       const std::string & param_name,
                       const ParameterType & param_type,
                       const std::vector<std::string> & param_value);

  /**
   * Convert a string to a numeric scalar and set it in the InputParameters object
   * @param params InputParameters object to set the parameter
   * @param param_name Name of the parameter to set
   * @param param_value Value of the parameter to set in string format
   * @param is_integer Flag to indicate if the parameter is an integer
   */
  template <typename T>
  void convertAndSetNumericScalar(InputParameters & params,
                                  const std::string & param_name,
                                  const std::string & param_value);

  /**
   * Convert a vector of strings to a numeric vector and set it in the InputParameters object
   * @param params InputParameters object to set the parameter
   * @param param_name Name of the parameter to set
   * @param param_value Value of the parameter to set in string format
   * @param is_integer Flag to indicate if the parameter is an integer
   */
  template <typename T>
  void convertAndSetNumericVector(InputParameters & params,
                                  const std::string & param_name,
                                  const std::vector<std::string> & param_value);
};
