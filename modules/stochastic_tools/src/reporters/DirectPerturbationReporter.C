//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirectPerturbationReporter.h"
#include "DirectPerturbationSampler.h"

registerMooseObject("StochasticToolsApp", DirectPerturbationReporter);

InputParameters
DirectPerturbationReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Compute local sensitivities using the direct perturbation method.");

  params.addRequiredParam<SamplerName>("sampler",
                                       "Direct Perturbation sampler used to generate samples.");
  params.addParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors",
      {},
      "List of VectorPostprocessor(s) to utilize for sensitivity computations.");
  params.addParam<std::vector<ReporterName>>(
      "reporters", {}, "List of Reporter values to utilize for sensitivity computations.");

  params.addParam<bool>("relative_sensitivity",
                        false,
                        "If the reporter should return relative or absolute sensitivities.");

  return params;
}

DirectPerturbationReporter::DirectPerturbationReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _sampler(getSampler<DirectPerturbationSampler>("sampler")),
    _relative_sensitivity(getParam<bool>("relative_sensitivity")),
    _initialized(false)
{
  if (getParam<std::vector<ReporterName>>("reporters").empty() &&
      getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors").empty())
    mooseError(
        "The 'vectorpostprocessors' and/or 'reporters' parameters must be defined and non-empty.");
}

void
DirectPerturbationReporter::initialize()
{
  // It is enough to do this once
  if (_initialized)
    return;

  // Sensitivities from Reporters
  std::vector<std::string> unsupported_types;
  for (const auto & r_name : getParam<std::vector<ReporterName>>("reporters"))
  {
    if (hasReporterValueByName<std::vector<Real>>(r_name))
      declareValueHelper<Real>(r_name);
    else if (hasReporterValueByName<std::vector<std::vector<Real>>>(r_name))
      declareValueHelper<std::vector<Real>>(r_name);
    else
      unsupported_types.push_back(r_name.getCombinedName());
  }

  if (!unsupported_types.empty())
    paramError("reporters",
               "The following reporter value(s) do not have a type supported by the "
               "DirectPerturbationReporter:\n",
               MooseUtils::join(unsupported_types, ", "));

  // Sensitivities from VPPs
  for (const auto & vpp_name :
       getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors"))
    for (const auto & vec_name :
         _fe_problem.getVectorPostprocessorObjectByName(vpp_name).getVectorNames())
      declareValueHelper<Real>(ReporterName(vpp_name, vec_name));
  _initialized = true;
}

template <typename DataType>
void
DirectPerturbationReporter::declareValueHelper(const ReporterName & r_name)
{
  // We create a reporter value for the sensitivities based on the input
  const auto & data = getReporterValueByName<std::vector<DataType>>(r_name);
  const std::string s_name = r_name.getObjectName() + "_" + r_name.getValueName();
  declareValueByName<std::vector<DataType>, DirectPerturbationReporterContext<DataType>>(
      s_name, REPORTER_MODE_ROOT, _sampler, data, _relative_sensitivity);
}

template <typename DataType>
DirectPerturbationReporterContext<DataType>::DirectPerturbationReporterContext(
    const libMesh::ParallelObject & other,
    const MooseObject & producer,
    ReporterState<std::vector<DataType>> & state,
    DirectPerturbationSampler & sampler,
    const std::vector<DataType> & data,
    const bool relative_sensitivity)
  : ReporterGeneralContext<std::vector<DataType>>(other, producer, state),
    _sampler(sampler),
    _data(data),
    _relative_sensitivity(relative_sensitivity)
{
  this->_state.value().resize(_sampler.getNumberOfCols());
}

template <typename DataType>
void
DirectPerturbationReporterContext<DataType>::finalize()
{
  const dof_id_type offset = _sampler.getLocalRowBegin();
  const dof_id_type num_columns = _sampler.getNumberOfCols();

  // If we are computing the relative sensitivity, we will need
  // the reference value. So the process that has that will communicate it
  // to everybody. We reuse the initialization function for the reference
  // value as well
  auto reference_value = initializeDataType(_data[0]);
  if (_relative_sensitivity)
  {
    if (_sampler.getLocalRowBegin() == 0)
      reference_value = _data[0];

    this->comm().sum(reference_value);
  }

  for (const auto param_i : make_range(num_columns))
  {
    // If we need relative coefficients we need to normalize the difference
    const auto interval = _relative_sensitivity ? _sampler.getRelativeInterval(param_i)
                                                : _sampler.getAbsoluteInterval(param_i);

    dof_id_type left_i;
    dof_id_type right_i;
    // Depending on which method we use, the indices will change
    if (_sampler.perturbationMethod() == "central_difference")
    {
      left_i = 2 * param_i + 1;
      right_i = 2 * param_i + 2;
    }
    else
    {
      left_i = param_i + 1;
      right_i = 0;
    }

    const bool left_i_in_owned_range =
        _sampler.getLocalRowBegin() <= left_i && left_i < _sampler.getLocalRowEnd();
    const bool right_i_in_owned_range =
        _sampler.getLocalRowBegin() <= right_i && right_i < _sampler.getLocalRowEnd();

    if (left_i_in_owned_range || right_i_in_owned_range)
    {
      const dof_id_type copy_i = left_i_in_owned_range ? left_i - offset : right_i - offset;
      this->_state.value()[param_i] = initializeDataType(_data[copy_i]);
    }

    // We add the contribution from one side
    if (left_i_in_owned_range)
      addSensitivityContribution(
          this->_state.value()[param_i], _data[left_i - offset], reference_value, interval);

    // We add the contribution from the other side
    if (right_i_in_owned_range)
      addSensitivityContribution(
          this->_state.value()[param_i], _data[right_i - offset], reference_value, -interval);
  }

  // We gather the contributions across all processors
  this->vectorSum();
}

template <typename DataType>
void
DirectPerturbationReporterContext<DataType>::addSensitivityContribution(
    DataType & add_to,
    const DataType & to_add,
    const DataType & reference_value,
    const Real interval) const
{
  // DataType is a numeric type that we can sum (excluding bool)
  if constexpr (std::is_arithmetic<DataType>::value && !std::is_same<DataType, bool>::value)
  {
    add_to += to_add / (_relative_sensitivity ? interval * reference_value : interval);
    return;
  }
  // DataType is a vector type
  else if constexpr (is_std_vector<DataType>::value)
  {
    // It can still be anything in the vector elements, so we will check this later
    using VectorValueType = typename DataType::value_type;

    mooseAssert(add_to.size() == to_add.size(), "The vectors for summation have different sizes!");

    // Check if the vector elements are of a numeric type
    if constexpr (std::is_arithmetic<VectorValueType>::value &&
                  !std::is_same<VectorValueType, bool>::value)
    {
      for (const auto index : index_range(add_to))
        add_to[index] +=
            to_add[index] / (_relative_sensitivity ? interval * reference_value[index] : interval);
      return;
    }
    // Check if the vector elements are also vectors
    else if constexpr (is_std_vector<VectorValueType>::value)
    {
      // This is as deep as we will go for now
      using InnerValueType = typename VectorValueType::value_type;

      // Check if the inner vector elements are of a numeric type
      if constexpr (std::is_arithmetic<InnerValueType>::value &&
                    !std::is_same<InnerValueType, bool>::value)
      {
        // Iterate over each inner vector in the outer vector
        for (auto & inner_index : index_range(add_to))
        {
          mooseAssert(add_to[inner_index].size() == to_add[inner_index].size(),
                      "The vectors for summation have different sizes!");
          for (const auto index : index_range(add_to[inner_index]))
            add_to[inner_index][index] +=
                to_add[inner_index][index] /
                (_relative_sensitivity ? interval * reference_value[inner_index][index] : interval);
        }
        return;
      }
      else
        static_assert(Moose::always_false<DataType>,
                      "Sensitivity coefficient computation is not implemented for the given type!");
    }
  }
  else
    static_assert(Moose::always_false<DataType>,
                  "Sensitivity coefficient computation is not implemented for the given type!");
}

template <typename DataType>
DataType
DirectPerturbationReporterContext<DataType>::initializeDataType(
    const DataType & example_output) const
{
  // DataType is a numeric type so we just return 0
  if constexpr (std::is_arithmetic<DataType>::value && !std::is_same<DataType, bool>::value)
    return 0.0;
  // DataType is a vector type
  else if constexpr (is_std_vector<DataType>::value)
  {
    // It can still be anything in the vector elements, so we will check this later
    using VectorValueType = typename DataType::value_type;

    // Check if the vector elements are of a numeric type
    if constexpr (std::is_arithmetic<VectorValueType>::value &&
                  !std::is_same<VectorValueType, bool>::value)
      return std::vector<VectorValueType>(example_output.size(), 0);
    // Check if the vector elements are also vectors
    else if constexpr (is_std_vector<VectorValueType>::value)
    {
      // This is as deep as we will go for now
      using InnerValueType = typename VectorValueType::value_type;

      // Check if the inner vector elements are of a numeric type
      if constexpr (std::is_arithmetic<InnerValueType>::value &&
                    !std::is_same<InnerValueType, bool>::value)
      {
        auto return_vector = std::vector<VectorValueType>(example_output.size());
        // Iterate over each inner vector in the outer vector
        for (auto & inner_index : index_range(example_output))
          return_vector[inner_index].resize(example_output.size(), 0.0);
        return return_vector;
      }
    }
    else
      static_assert(Moose::always_false<DataType>,
                    "Sensitivity coefficient computation is not implemented for the given type!");
  }
  else
    static_assert(Moose::always_false<DataType>,
                  "Sensitivity coefficient initialization is not implemented for the given type!");
}
