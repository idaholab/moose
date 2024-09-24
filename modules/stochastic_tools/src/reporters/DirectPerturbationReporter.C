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
                                       "Direct PErturbation sampler used to generate samples.");
  params.addParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors",
      "List of VectorPostprocessor(s) to utilized for statistic computations.");
  params.addParam<std::vector<ReporterName>>(
      "reporters", {}, "List of Reporter values to utilized for statistic computations.");

  return params;
}

DirectPerturbationReporter::DirectPerturbationReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _sampler(getSampler<DirectPerturbationSampler>("sampler")),
    _initialized(false)
{
  if ((!isParamValid("reporters") && !isParamValid("vectorpostprocessors")) ||
      (getParam<std::vector<ReporterName>>("reporters").empty() &&
       getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors").empty()))
    mooseError(
        "The 'vectorpostprocessors' and/or 'reporters' parameters must be defined and non-empty.");
}

void
DirectPerturbationReporter::initialize()
{
  if (_initialized)
    return;

  // Stats for Reporters
  if (isParamValid("reporters"))
  {
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
  }

  // Stats for VPP
  if (isParamValid("vectorpostprocessors"))
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
  const auto & data = getReporterValueByName<std::vector<DataType>>(r_name);
  const std::string s_name = r_name.getObjectName() + "_" + r_name.getValueName();
  declareValueByName<std::vector<DataType>, DirectPerturbationReporterContext<DataType>>(
      s_name, REPORTER_MODE_ROOT, _sampler, data);
}

template <typename DataType>
DirectPerturbationReporterContext<DataType>::DirectPerturbationReporterContext(
    const libMesh::ParallelObject & other,
    const MooseObject & producer,
    ReporterState<std::vector<DataType>> & state,
    DirectPerturbationSampler & sampler,
    const std::vector<DataType> & data)
  : ReporterGeneralContext<std::vector<DataType>>(other, producer, state),
    _sampler(sampler),
    _data(data)
{
  this->_state.value().resize(_sampler.getNumberOfCols());
}

template <typename DataType>
void
DirectPerturbationReporterContext<DataType>::finalize()
{
  dof_id_type offset;
  if (_data.size() == _sampler.getNumberOfRows())
    offset = _sampler.getLocalRowBegin();
  else if (_data.size() == _sampler.getNumberOfLocalRows())
    offset = 0;
  else
    mooseError("Data size inconsistency. Expected data vector to have ",
               _sampler.getNumberOfLocalRows(),
               " (local) or ",
               _sampler.getNumberOfRows(),
               " (global) elements, but actually has ",
               _data.size(),
               " elements. Are you using the same sampler?");

  const dof_id_type num_columns = _sampler.getNumberOfCols();

  DataType value(_sampler.getNumberOfCols());

  for (const auto param_i : make_range(num_columns))
  {
    const auto & interval = _sampler.getInterval(param_i);

    const auto left_i = param_i;
    dof_id_type right_i;
    if (_sampler.perturbationMethod() == "central_difference")
      right_i = param_i + 1;
    else
      right_i = num_columns - 1;

    if (_sampler.getLocalRowBegin() <= left_i && left_i < _sampler.getLocalRowEnd())
      value[param_i] += _data[left_i];

    if (_sampler.getLocalRowBegin() <= left_i && left_i < _sampler.getLocalRowEnd())
      value[param_i] -= _data[right_i];

    value[param_i] = value[param_i] / interval;
  }

  this->comm().sum(value);
  this->_state.value() = value;
}
