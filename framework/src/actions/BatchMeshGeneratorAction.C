//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "BatchMeshGeneratorAction.h"
#include "ActionFactory.h"

#include "MooseMesh.h"
#include "MeshGenerator.h"
#include "Factory.h"
#include "MooseApp.h"

#include "hit/hit.h"

registerMooseAction("MooseApp", BatchMeshGeneratorAction, "add_mesh_generator");

InputParameters
BatchMeshGeneratorAction::validParams()
{
  InputParameters params = Action::validParams();

  params.set<std::string>("type") = "BatchMeshGeneratorAction";

  params.addClassDescription("Batch generate meshes using actions.");
  params.addRequiredParam<std::string>("mesh_generator_type",
                                       "Type of the mesh generator to be batch generated.");
  params.addRequiredParam<std::string>("mesh_name_prefix",
                                       "Prefix name of the meshes to be batch generated.");

  params.addParam<std::vector<std::string>>("batch_scalar_input_param_names",
                                            std::vector<std::string>(),
                                            "Names of the scalar input parameters to be altered.");
  MultiMooseEnum default_types("BOOL REAL SHORT USHORT INT UINT STRING ENUM", "");
  params.addParam<MultiMooseEnum>(
      "batch_scalar_input_param_types",
      default_types,
      "Types of the scalar input parameters to be altered in each generator of the batch.");
  params.addParam<std::vector<std::vector<std::string>>>(
      "batch_scalar_input_param_values",
      {},
      "Values of the scalar input parameters to be assigned.");

  params.addParam<std::vector<std::string>>("batch_vector_input_param_names",
                                            std::vector<std::string>(),
                                            "Name of the vector input parameters to be altered.");
  params.addParam<MultiMooseEnum>("batch_vector_input_param_types",
                                  default_types,
                                  "Type of the vector input parameters to be altered.");
  params.addParam<std::vector<std::vector<std::vector<std::string>>>>(
      "batch_vector_input_param_values",
      {},
      "Values of the vector input parameters to be assigned.");

  MooseEnum multi_batch_params_method("corresponding cartesian_product", "cartesian_product");
  params.addParam<MooseEnum>("multi_batch_params_method",
                             multi_batch_params_method,
                             "Method to generate multiple batch parameters.Options: " +
                                 multi_batch_params_method.getRawNames());

  params.addParam<std::vector<std::string>>(
      "fixed_scalar_input_param_names", {}, "Names of the input parameters to be fixed.");
  params.addParam<MultiMooseEnum>("fixed_scalar_input_param_types",
                                  default_types,
                                  "Types of the input parameters to be fixed.");
  params.addParam<std::vector<std::string>>(
      "fixed_scalar_input_param_values", {}, "Values of the input parameters to be fixed.");
  params.addParam<std::vector<std::string>>(
      "fixed_vector_input_param_names", {}, "Names of the input vector parameters to be fixed.");
  params.addParam<MultiMooseEnum>("fixed_vector_input_param_types",
                                  default_types,
                                  "Types of the input vector parameters to be fixed.");
  params.addParam<std::vector<std::vector<std::string>>>(
      "fixed_vector_input_param_values", {}, "Values of the input vector parameters to be fixed.");
  params.addParam<bool>("use_decomposed_index",
                        false,
                        "Whether to use the decomposed index for the mesh name (only effective for "
                        "the cartesian_product method).");

  params.addParamNamesToGroup("batch_scalar_input_param_names batch_scalar_input_param_types "
                              "batch_scalar_input_param_values "
                              "batch_vector_input_param_names batch_vector_input_param_types "
                              "batch_vector_input_param_values",
                              "Batch Input");
  params.addParamNamesToGroup("fixed_scalar_input_param_names fixed_scalar_input_param_types "
                              "fixed_scalar_input_param_values "
                              "fixed_vector_input_param_names fixed_vector_input_param_types "
                              "fixed_vector_input_param_values",
                              "Fixed Input");

  return params;
}

BatchMeshGeneratorAction::BatchMeshGeneratorAction(const InputParameters & params)
  : Action(params),
    InputParametersChecksUtils<BatchMeshGeneratorAction>(this),
    _mesh_generator_type(getParam<std::string>("mesh_generator_type")),
    _mesh_name_prefix(getParam<std::string>("mesh_name_prefix")),
    _batch_scalar_input_param_names(
        getParam<std::vector<std::string>>("batch_scalar_input_param_names")),
    _batch_scalar_input_param_types(isParamValid("batch_scalar_input_param_types")
                                        ? getParam<MultiMooseEnum>("batch_scalar_input_param_types")
                                              .template getSetValueIDs<ParameterType>()
                                        : std::vector<ParameterType>()),
    _batch_scalar_input_param_values(
        getParam<std::vector<std::vector<std::string>>>("batch_scalar_input_param_values")),
    _batch_vector_input_param_names(
        getParam<std::vector<std::string>>("batch_vector_input_param_names")),
    _batch_vector_input_param_types(isParamValid("batch_vector_input_param_types")
                                        ? getParam<MultiMooseEnum>("batch_vector_input_param_types")
                                              .template getSetValueIDs<ParameterType>()
                                        : std::vector<ParameterType>()),
    _batch_vector_input_param_values(getParam<std::vector<std::vector<std::vector<std::string>>>>(
        "batch_vector_input_param_values")),
    _multi_batch_params_method(getParam<MooseEnum>("multi_batch_params_method")
                                   .template getEnum<MultiBatchParamsMethod>()),
    _fixed_scalar_input_param_names(
        getParam<std::vector<std::string>>("fixed_scalar_input_param_names")),
    _fixed_scalar_input_param_types(isParamValid("fixed_scalar_input_param_types")
                                        ? getParam<MultiMooseEnum>("fixed_scalar_input_param_types")
                                              .template getSetValueIDs<ParameterType>()
                                        : std::vector<ParameterType>()),
    _fixed_scalar_input_param_values(
        getParam<std::vector<std::string>>("fixed_scalar_input_param_values")),
    _fixed_vector_input_param_names(
        getParam<std::vector<std::string>>("fixed_vector_input_param_names")),
    _fixed_vector_input_param_types(isParamValid("fixed_vector_input_param_types")
                                        ? getParam<MultiMooseEnum>("fixed_vector_input_param_types")
                                              .template getSetValueIDs<ParameterType>()
                                        : std::vector<ParameterType>()),
    _fixed_vector_input_param_values(
        getParam<std::vector<std::vector<std::string>>>("fixed_vector_input_param_values")),
    _use_decomposed_index(getParam<bool>("use_decomposed_index"))
{
  // Sanity check for the fixed input parameters
  checkVectorParamAndMultiMooseEnumLength<std::string>("fixed_scalar_input_param_names",
                                                       "fixed_scalar_input_param_types");
  checkVectorParamsSameLength<std::string, std::string>("fixed_scalar_input_param_names",
                                                        "fixed_scalar_input_param_values");
  checkVectorParamAndMultiMooseEnumLength<std::string>("fixed_vector_input_param_names",
                                                       "fixed_vector_input_param_types");
  checkVectorParamsSameLength<std::string, std::vector<std::string>>(
      "fixed_vector_input_param_names", "fixed_vector_input_param_values");

  // Sanity check for the batch input parameters
  checkVectorParamAndMultiMooseEnumLength<std::string>("batch_scalar_input_param_names",
                                                       "batch_scalar_input_param_types");
  checkVectorParamsSameLength<std::string, std::vector<std::string>>(
      "batch_scalar_input_param_names", "batch_scalar_input_param_values");
  checkVectorParamAndMultiMooseEnumLength<std::string>("batch_vector_input_param_names",
                                                       "batch_vector_input_param_types");
  checkVectorParamsSameLength<std::string, std::vector<std::vector<std::string>>>(
      "batch_vector_input_param_names", "batch_vector_input_param_values");

  // At least we want this action to create one mesh generator
  if (_batch_scalar_input_param_names.empty() && _batch_vector_input_param_names.empty())
    mooseError("BatchMeshGeneratorAction: batch_scalar_input_param_names and "
               "batch_vector_input_param_names cannot be empty at the same time.");

  // If the previous check is passed, batch_params_sizes will not be empty
  // But we need to check if any element of the batch_params_values are empty
  std::set<unsigned int> batch_params_sizes;
  for (const auto & unit_batch_scalar_param_values : _batch_scalar_input_param_values)
  {
    if (unit_batch_scalar_param_values.empty())
      paramError("batch_scalar_input_param_values",
                 "this parameter cannot contain empty elements.");
    batch_params_sizes.emplace(unit_batch_scalar_param_values.size());
  }
  for (const auto & unit_batch_vector_param_values : _batch_vector_input_param_values)
  {
    if (unit_batch_vector_param_values.empty())
      paramError("batch_vector_input_param_values",
                 "this parameter cannot contain empty elements.");
    batch_params_sizes.emplace(unit_batch_vector_param_values.size());
  }

  // Then for the corresponding method, the sizes of the batch_params_values should be the same
  if (_multi_batch_params_method == MultiBatchParamsMethod::corresponding &&
      batch_params_sizes.size() > 1)
    mooseError("BatchMeshGeneratorAction: elements of batch_scalar_input_param_values and "
               "batch_vector_input_param_values must have the same size.");

  // Decomposed index cannot be used with the corresponding method
  if (_use_decomposed_index && _multi_batch_params_method == MultiBatchParamsMethod::corresponding)
    paramError("use_decomposed_index",
               "Decomposed index cannot be used with the corresponding method.");
  // Check the parameter types are given correctly here so that we do not need to do it when setting
  // the values.
  auto set_params = _app.getFactory().getValidParams(_mesh_generator_type);
  // batch scalar input parameters
  checkInputParametersTypes(set_params,
                            "batch_scalar_input_param_names",
                            _batch_scalar_input_param_names,
                            _batch_scalar_input_param_types);
  // fix scalar input parameters
  checkInputParametersTypes(set_params,
                            "fixed_scalar_input_param_names",
                            _fixed_scalar_input_param_names,
                            _fixed_scalar_input_param_types);
  // batch vector input parameters
  checkInputParametersTypes(set_params,
                            "batch_vector_input_param_names",
                            _batch_vector_input_param_names,
                            _batch_vector_input_param_types,
                            true);
  // fix vector input parameters
  checkInputParametersTypes(set_params,
                            "fixed_vector_input_param_names",
                            _fixed_vector_input_param_names,
                            _fixed_vector_input_param_types,
                            true);
}

void
BatchMeshGeneratorAction::act()
{
  if (_current_task == "add_mesh_generator")
    addMeshGenerators();
}

void
BatchMeshGeneratorAction::addMeshGenerators()
{
  std::vector<std::vector<std::string>> processed_batch_scalar_input_param_values;
  std::vector<std::vector<std::vector<std::string>>> processed_batch_vector_input_param_values;
  // generate the decomposed indices for the cartesian product method
  std::vector<std::vector<unsigned int>> processed_batch_indices;
  if (_multi_batch_params_method == MultiBatchParamsMethod::corresponding)
  {
    processed_batch_scalar_input_param_values = _batch_scalar_input_param_values;
    processed_batch_vector_input_param_values = _batch_vector_input_param_values;
    if (_use_decomposed_index)
    {
      processed_batch_indices.push_back(std::vector<unsigned int>(
          processed_batch_vector_input_param_values.empty()
              ? processed_batch_scalar_input_param_values.front().size()
              : processed_batch_vector_input_param_values.front().size()));
      std::iota(processed_batch_indices.back().begin(), processed_batch_indices.back().end(), 0);
    }
  }
  else // cartesian_product
  {
    // We basically need to reconstruct the corresponding parameters based on the cartesian product
    // algorithm
    for (const auto i : index_range(_batch_scalar_input_param_values))
    {
      // For the first element, just copy the parameters
      if (processed_batch_scalar_input_param_values.empty())
      {
        processed_batch_scalar_input_param_values.push_back(_batch_scalar_input_param_values[i]);
        if (_use_decomposed_index)
        {
          processed_batch_indices.push_back(
              std::vector<unsigned int>(_batch_scalar_input_param_values[i].size()));
          std::iota(
              processed_batch_indices.back().begin(), processed_batch_indices.back().end(), 0);
        }
      }
      else
      {
        const unsigned int num_new_batch_params = _batch_scalar_input_param_values[i].size();
        const unsigned int num_processed_batch_params =
            processed_batch_scalar_input_param_values.front().size();
        // All the elements in the processed_batch_scalar_input_param_values need to be duplicated
        // for num_new_batch_params times
        for (auto & unit_processed_batch_scalar_input_param_values :
             processed_batch_scalar_input_param_values)
        {
          auto temp_params = unit_processed_batch_scalar_input_param_values;
          for (unsigned int j = 1; j < num_new_batch_params; j++)
          {
            unit_processed_batch_scalar_input_param_values.insert(
                unit_processed_batch_scalar_input_param_values.end(),
                temp_params.begin(),
                temp_params.end());
          }
        }
        if (_use_decomposed_index)
        {
          // Same as the composed indices
          for (auto & unit_processed_batch_indices : processed_batch_indices)
          {
            auto temp_indices = unit_processed_batch_indices;
            for (unsigned int j = 1; j < num_new_batch_params; j++)
            {
              unit_processed_batch_indices.insert(
                  unit_processed_batch_indices.end(), temp_indices.begin(), temp_indices.end());
            }
          }
        }

        // Then, add a new element to the processed_batch_scalar_input_param_values by repeating
        // each element in _batch_scalar_input_param_values[i] for num_processed_batch_params times
        processed_batch_scalar_input_param_values.push_back({});
        for (const auto & unit_batch_scalar_input_param_values :
             _batch_scalar_input_param_values[i])
          for (unsigned int j = 0; j < num_processed_batch_params; j++)
            processed_batch_scalar_input_param_values.back().push_back(
                unit_batch_scalar_input_param_values);
        if (_use_decomposed_index)
        {
          // Same as the composed indices
          processed_batch_indices.push_back({});
          for (const auto & unit_batch_scalar_input_param_values_index :
               index_range(_batch_scalar_input_param_values[i]))
            for (unsigned int j = 0; j < num_processed_batch_params; j++)
              processed_batch_indices.back().push_back(unit_batch_scalar_input_param_values_index);
        }
      }
    }
    for (const auto i : index_range(_batch_vector_input_param_values))
    {
      // For the first element, just copy the parameters
      if (processed_batch_vector_input_param_values.empty())
      {
        if (processed_batch_scalar_input_param_values.empty())
        {
          // if no batch scalar input parameters are used
          // we just need to initiate the processed_batch_vector_input_param_values as the first one
          // to fill
          processed_batch_vector_input_param_values.push_back(_batch_vector_input_param_values[i]);
          if (_use_decomposed_index)
          {
            processed_batch_indices.push_back(
                std::vector<unsigned int>(_batch_vector_input_param_values[i].size()));
            std::iota(
                processed_batch_indices.back().begin(), processed_batch_indices.back().end(), 0);
          }
        }
        else
        {
          processed_batch_vector_input_param_values.push_back({});
          // if there are batch scalar input parameters, then each element needs to be duplicated
          // for that amount of times
          for (const auto & unit_batch_vector_input_param_values :
               _batch_vector_input_param_values[i])
            for (unsigned int j = 0; j < processed_batch_scalar_input_param_values.front().size();
                 j++)
              processed_batch_vector_input_param_values.back().push_back(
                  unit_batch_vector_input_param_values);
          // Then the scalar input parameters need to be duplicated for the number of elements in
          // the processed_batch_vector_input_param_values
          for (auto & unit_processed_batch_scalar_input_param_values :
               processed_batch_scalar_input_param_values)
          {
            auto temp_params = unit_processed_batch_scalar_input_param_values;
            for (unsigned int j = 1; j < processed_batch_vector_input_param_values.back().size();
                 j++)
            {
              unit_processed_batch_scalar_input_param_values.insert(
                  unit_processed_batch_scalar_input_param_values.end(),
                  temp_params.begin(),
                  temp_params.end());
            }
          }
          if (_use_decomposed_index)
          {
            // Add the indices for the first batch vector input parameter
            processed_batch_indices.push_back({});
            for (const auto & unit_batch_vector_input_param_values_index :
                 index_range(_batch_vector_input_param_values[i]))
              for (unsigned int j = 0; j < processed_batch_indices.front().size(); j++)
                processed_batch_indices.back().push_back(
                    unit_batch_vector_input_param_values_index);
            // Duplicate the indices for the batch scalar input parameters
            for (unsigned int k = 1; k < processed_batch_indices.size(); k++)
            {
              auto & unit_processed_batch_indices = processed_batch_indices[k - 1];
              auto temp_indices = unit_processed_batch_indices;
              for (unsigned int j = 1; j < _batch_vector_input_param_values[i].size(); j++)
              {
                unit_processed_batch_indices.insert(
                    unit_processed_batch_indices.end(), temp_indices.begin(), temp_indices.end());
              }
            }
          }
        }
      }
      else
      {
        const unsigned int num_new_batch_params = _batch_vector_input_param_values[i].size();
        const unsigned int num_processed_batch_params =
            processed_batch_vector_input_param_values.front().size();
        // All the elements in the processed_batch_vector_input_param_values need to be duplicated
        // for num_new_batch_params times
        for (auto & unit_processed_batch_vector_input_param_values :
             processed_batch_vector_input_param_values)
        {
          auto temp_params = unit_processed_batch_vector_input_param_values;
          for (unsigned int j = 1; j < num_new_batch_params; j++)
          {
            unit_processed_batch_vector_input_param_values.insert(
                unit_processed_batch_vector_input_param_values.end(),
                temp_params.begin(),
                temp_params.end());
          }
        }
        if (_use_decomposed_index)
        {
          // Same for the decomposed indices
          for (auto & unit_processed_batch_indices : processed_batch_indices)
          {
            auto temp_indices = unit_processed_batch_indices;
            for (unsigned int j = 1; j < num_new_batch_params; j++)
            {
              unit_processed_batch_indices.insert(
                  unit_processed_batch_indices.end(), temp_indices.begin(), temp_indices.end());
            }
          }
        }
        // if there are also batch scalar input parameters, it also needs to be duplicated
        for (auto & unit_processed_batch_scalar_input_param_values :
             processed_batch_scalar_input_param_values)
        {
          auto temp_params = unit_processed_batch_scalar_input_param_values;
          for (unsigned int j = 1; j < num_new_batch_params; j++)
          {
            unit_processed_batch_scalar_input_param_values.insert(
                unit_processed_batch_scalar_input_param_values.end(),
                temp_params.begin(),
                temp_params.end());
          }
        }
        // Then, add a new element to the processed_batch_vector_input_param_values by repeating
        // each element in _batch_vector_input_param_values[i] for num_processed_batch_params times
        processed_batch_vector_input_param_values.push_back({});
        for (const auto & unit_batch_vector_input_param_values :
             _batch_vector_input_param_values[i])
          for (unsigned int j = 0; j < num_processed_batch_params; j++)
            processed_batch_vector_input_param_values.back().push_back(
                unit_batch_vector_input_param_values);
        if (_use_decomposed_index)
        {
          // Same for the decomposed indices
          processed_batch_indices.push_back({});
          for (const auto & unit_batch_vector_input_param_values_index :
               index_range(_batch_vector_input_param_values[i]))
            for (unsigned int j = 0; j < num_processed_batch_params; j++)
              processed_batch_indices.back().push_back(unit_batch_vector_input_param_values_index);
        }
      }
    }
  }

  // Now, we can add the mesh generators by looping through the processed params
  const unsigned int num_batch_params =
      processed_batch_vector_input_param_values.empty()
          ? processed_batch_scalar_input_param_values.front().size()
          : processed_batch_vector_input_param_values.front().size();
  for (const auto i : make_range(num_batch_params))
  {
    auto params = _app.getFactory().getValidParams(_mesh_generator_type);
    for (const auto j : index_range(_batch_scalar_input_param_values))
      setScalarParams(params,
                      _batch_scalar_input_param_names[j],
                      _batch_scalar_input_param_types[j],
                      processed_batch_scalar_input_param_values[j][i]);
    for (const auto j : index_range(_batch_vector_input_param_values))
      setVectorParams(params,
                      _batch_vector_input_param_names[j],
                      _batch_vector_input_param_types[j],
                      processed_batch_vector_input_param_values[j][i]);
    for (const auto j : index_range(_fixed_scalar_input_param_names))
      setScalarParams(params,
                      _fixed_scalar_input_param_names[j],
                      _fixed_scalar_input_param_types[j],
                      _fixed_scalar_input_param_values[j]);
    for (const auto j : index_range(_fixed_vector_input_param_names))
      setVectorParams(params,
                      _fixed_vector_input_param_names[j],
                      _fixed_vector_input_param_types[j],
                      _fixed_vector_input_param_values[j]);

    std::string mesh_index;
    if (_use_decomposed_index)
      for (const auto & process_batch_index : processed_batch_indices)
        mesh_index += '_' + std::to_string(process_batch_index[i]);
    else
      mesh_index = "_" + std::to_string(i);

    _app.getMeshGeneratorSystem().addMeshGenerator(
        _mesh_generator_type, _mesh_name_prefix + mesh_index, params);
  }
}

void
BatchMeshGeneratorAction::setScalarParams(InputParameters & params,
                                          const std::string & param_name,
                                          const ParameterType & param_type,
                                          const std::string & param_value)
{
  switch (param_type)
  {
    case (ParameterType::REAL):
      params.set<Real>(param_name) = MooseUtils::convert<Real>(param_value);
      break;
    case (ParameterType::SHORT):
      params.set<short>(param_name) = MooseUtils::convert<short>(param_value);
      break;
    case (ParameterType::USHORT):
      params.set<unsigned short>(param_name) = MooseUtils::convert<unsigned short>(param_value);
      break;
    case (ParameterType::INT):
      params.set<int>(param_name) = MooseUtils::convert<int>(param_value);
      break;
    case (ParameterType::UINT):
      params.set<unsigned int>(param_name) = MooseUtils::convert<unsigned int>(param_value);
      break;
    case (ParameterType::ENUM):
      params.set<MooseEnum>(param_name) = param_value;
      break;
    case (ParameterType::STRING):
      params.set<std::string>(param_name) = param_value;
      break;
    case (ParameterType::BOOL):
      hit::toBool(param_value, &params.set<bool>(param_name));
      break;
    default:
      mooseAssert(false,
                  "impossible situation."); // as we use MultiMooseEnum to ensure the type is valid
  }
}

void
BatchMeshGeneratorAction::setVectorParams(InputParameters & params,
                                          const std::string & param_name,
                                          const ParameterType & param_type,
                                          const std::vector<std::string> & param_value)
{
  switch (param_type)
  {
    case (ParameterType::REAL):
      convertAndSetNumericVector<Real>(params, param_name, param_value);
      break;
    case (ParameterType::SHORT):
      convertAndSetNumericVector<short>(params, param_name, param_value);
      break;
    case (ParameterType::USHORT):
      convertAndSetNumericVector<unsigned short>(params, param_name, param_value);
      break;
    case (ParameterType::INT):
      convertAndSetNumericVector<int>(params, param_name, param_value);
      break;
    case (ParameterType::UINT):
      convertAndSetNumericVector<unsigned int>(params, param_name, param_value);
      break;
    case (ParameterType::ENUM):
      params.set<MultiMooseEnum>(param_name) = param_value;
      break;
    case (ParameterType::STRING):
      params.set<std::vector<std::string>>(param_name) = param_value;
      break;
    case (ParameterType::BOOL):
    {
      std::vector<bool> values(param_value.size());
      std::transform(param_value.begin(),
                     param_value.end(),
                     values.begin(),
                     [](const std::string & val)
                     {
                       bool tmp;
                       hit::toBool(val, &tmp);
                       return tmp;
                     });
      params.set<std::vector<bool>>(param_name) = values;
      break;
    }
    default:
      mooseAssert(false,
                  "impossible situation."); // as we use MultiMooseEnum to ensure the type is valid
  }
}

template <typename T>
void
BatchMeshGeneratorAction::convertAndSetNumericVector(InputParameters & params,
                                                     const std::string & param_name,
                                                     const std::vector<std::string> & param_value)
{
  std::vector<T> values(param_value.size());
  std::transform(param_value.begin(),
                 param_value.end(),
                 values.begin(),
                 [](const std::string & val) { return MooseUtils::convert<T>(val); });
  params.set<std::vector<T>>(param_name) = values;
}

void
BatchMeshGeneratorAction::checkInputParametersTypes(const InputParameters & params,
                                                    const std::string & action_input_param_name,
                                                    const std::vector<std::string> & param_names,
                                                    const std::vector<ParameterType> & param_types,
                                                    const bool & is_vector)
{
  for (const auto i : index_range(param_names))
  {
    switch (param_types[i])
    {
      case (ParameterType::REAL):
        checkInputParameterType<Real>(params, action_input_param_name, param_names[i], is_vector);
        break;
      case (ParameterType::SHORT):
        checkInputParameterType<short>(params, action_input_param_name, param_names[i], is_vector);
        break;
      case (ParameterType::USHORT):
        checkInputParameterType<unsigned short>(
            params, action_input_param_name, param_names[i], is_vector);
        break;
      case (ParameterType::INT):
        checkInputParameterType<int>(params, action_input_param_name, param_names[i], is_vector);
        break;
      case (ParameterType::UINT):
        checkInputParameterType<unsigned int>(
            params, action_input_param_name, param_names[i], is_vector);
        break;
      case (ParameterType::ENUM):
        if (is_vector)
          checkInputParameterType<MultiMooseEnum>(
              params, action_input_param_name, param_names[i], false);
        else
          checkInputParameterType<MooseEnum>(
              params, action_input_param_name, param_names[i], false);
        break;
      case (ParameterType::STRING):
        checkInputParameterType<std::string>(
            params, action_input_param_name, param_names[i], is_vector);
        break;
      case (ParameterType::BOOL):
        checkInputParameterType<bool>(params, action_input_param_name, param_names[i], is_vector);
        break;
      default:
        mooseAssert(
            false,
            "impossible situation."); // as we use MultiMooseEnum to ensure the type is valid
    }
  }
}

template <typename T>
void
BatchMeshGeneratorAction::checkInputParameterType(const InputParameters & params,
                                                  const std::string & action_input_param_name,
                                                  const std::string & param_name,
                                                  const bool & is_vector)
{
  if ((is_vector && !params.isType<std::vector<T>>(param_name)) ||
      (!is_vector && !params.isType<T>(param_name)))
    paramError(action_input_param_name,
               "the input parameter, " + param_name + ", has the wrong type. It should be " +
                   params.type(param_name) + ".");
}
