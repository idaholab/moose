//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticReporter.h"

template <typename T>
class ActiveLearningReporterTempl;

/**
 * This is a base class for performing active learning routines, meant to be used
 * in conjunction with Sampler multiapps and SamplerReporterTransfer. The purpose
 * is to determine if a sample needs to be evaluated by a multiphysics solve and
 * possibly replace quantities of interest with values computed by other means.
 */
template <typename T>
class ActiveLearningReporterTempl : public StochasticReporter
{
public:
  static InputParameters validParams();

  ActiveLearningReporterTempl(const InputParameters & parameters);

  /**
   * Here we loop through the samples and call the needSample function to determine
   * if the sample needs to be run and define a value in its place.
   */
  virtual void execute() override;

protected:
  /**
   * This is overriden for the following reasons:
   *   1) Only one vector can be declared and must match the type of this class.
   *   2) Check that the inputted sampler matches the one given in the parameters.
   *   3) We actually get a pointer to the declared value so we can replace it
   *      (if necessary) in the needSample routine.
   *   4) Declare the "need_sample" value which can be used to evaluate the sample
   *      or not.
   */
  virtual ReporterName declareStochasticReporterClone(const Sampler & sampler,
                                                      const ReporterData & from_data,
                                                      const ReporterName & from_reporter,
                                                      std::string prefix = "") override;

  /**
   * Get a const reference to the sampler from the parameters. This is preferred
   * over having _sampler being a protected member since we don't want derived classes
   * changing the state of the sampler during the loop in execute.
   */
  const Sampler & sampler() const { return _sampler; }

  const std::vector<std::vector<Real>> & getGlobalInputData() const
  {
    _input_data_requested = true;
    return _input_data;
  }

  /**
   * Get a const reference to the output data
   */
  const std::vector<T> & getGlobalOutputData() const
  {
    _output_data_requested = true;
    return _output_data;
  }

  /**
   * Optional virtual function that is called before the sampler loop calling needSample
   */
  virtual void preNeedSample() {}

  /**
   * This routine is called during the sampler loop in execute() and is meant to fill
   * in the "need_sample" reporter value and modify the data declared by the transfer.
   *
   * @param row The row of data from the sampler
   * @param local_ind The local index of the sampler row
   * @param global_ind The global index of the sampler row
   * @param val Reference to the value associated with the row of data.
   * @return Filled in value of "need_sample", meant to determine if a sample needs
   *         to be run by a multiapp or other means.
   */
  virtual bool needSample(const std::vector<Real> & /*row*/,
                          dof_id_type /*local_ind*/,
                          dof_id_type /*global_ind*/,
                          T & /*val*/)
  {
    return true;
  }

private:
  /// Sampler given in the parameters, must match the one used to declare the
  /// transferred values.
  Sampler & _sampler;
  /// Reporter value determining whether we need to evaluate the sample through
  /// a multiapp or other means.
  std::vector<bool> & _need_sample;
  /// Reporter value declared with the transfer
  std::vector<T> * _data = nullptr;

  /// Whether or not to gather global input data
  mutable bool _input_data_requested = false;
  /// Whether or not to gather global output data
  mutable bool _output_data_requested = false;
  /// Global input data from sampler
  std::vector<std::vector<Real>> _input_data;
  /// Global output data from sampler
  std::vector<T> _output_data;
};

template <typename T>
InputParameters
ActiveLearningReporterTempl<T>::validParams()
{
  InputParameters params = StochasticReporter::validParams();
  params.addRequiredParam<SamplerName>("sampler", "The sampler used to produce data.");
  return params;
}

template <typename T>
ActiveLearningReporterTempl<T>::ActiveLearningReporterTempl(const InputParameters & parameters)
  : StochasticReporter(parameters),
    _sampler(this->template getSampler<Sampler>("sampler")),
    _need_sample(this->template declareStochasticReporter<bool>("need_sample", _sampler))
{
}

template <typename T>
void
ActiveLearningReporterTempl<T>::execute()
{
  // If requesting global data, fill it in
  if (_input_data_requested)
  {
    // Gather inputs for the current step
    _input_data.assign(_sampler.getNumberOfRows(),
                       std::vector<Real>(_sampler.getNumberOfCols(), 0.0));
    for (dof_id_type i = _sampler.getLocalRowBegin(); i < _sampler.getLocalRowEnd(); ++i)
      _input_data[i] = _sampler.getNextLocalRow();
    for (auto & inp : _input_data)
      gatherSum(inp);
  }
  if (_output_data_requested)
  {
    if (!_data)
      mooseError("Output data has been requested, but none was declared in this object.");
    _output_data = *_data;
    _communicator.allgather(_output_data);
  }

  // Optional call for before sampler loop
  preNeedSample();

  // Dummy value in case _data has not been declared yet
  T dummy;
  // Loop over samples to determine if sample is needed. Replace value in _data
  // (typically only if a sample is not needed). We insert a dummy value in case
  // _data has not been declared.
  for (const auto & i : make_range(_sampler.getNumberOfLocalRows()))
    _need_sample[i] = needSample(_sampler.getNextLocalRow(),
                                 i,
                                 i + _sampler.getLocalRowBegin(),
                                 (_data ? (*_data)[i] : dummy));
}

template <typename T>
ReporterName
ActiveLearningReporterTempl<T>::declareStochasticReporterClone(const Sampler & sampler,
                                                               const ReporterData & from_data,
                                                               const ReporterName & from_reporter,
                                                               std::string prefix)
{
  // Only one value is allowed to be declared
  if (_data)
    this->mooseError(type(), " can only declare a single reporter value.");
  // Make sure the inputted sampler is the same one in the parameters
  else if (sampler.name() != _sampler.name())
    this->paramError("sampler",
                     "Inputted sampler, ",
                     _sampler.name(),
                     ", is not the same as the one producing data, ",
                     sampler.name(),
                     ".");
  // Make sure reporter value exists
  else if (!from_data.hasReporterValue(from_reporter))
    this->mooseError("Reporter value ", from_reporter, " has not been declared.");
  // Make sure the reporter value is the right type
  else if (!from_data.hasReporterValue<T>(from_reporter))
    this->mooseError(
        type(), " can only use reporter values of type ", MooseUtils::prettyCppType<T>(), ".");

  std::string value_name = (prefix.empty() ? "" : prefix + ":") + from_reporter.getObjectName() +
                           ":" + from_reporter.getValueName();
  _data = &this->declareStochasticReporter<T>(value_name, sampler);
  return {name(), value_name};
}
