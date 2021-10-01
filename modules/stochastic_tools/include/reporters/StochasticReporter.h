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
#include "GeneralReporter.h"
#include "StochasticToolsUtils.h"
#include "Sampler.h"

template <typename T>
class StochasticReporterContext : public ReporterGeneralContext<std::vector<T>>
{
public:
  StochasticReporterContext(const libMesh::ParallelObject & other,
                            const MooseObject & producer,
                            ReporterState<std::vector<T>> & state,
                            const Sampler & sampler);

  virtual void copyValuesBack() override;
  virtual void finalize() override;
  virtual std::string contextType() const override { return MooseUtils::prettyCppType(this); }
  virtual void storeInfo(nlohmann::json & json) const override;

protected:
  const Sampler & _sampler;
  bool _has_gathered;
  bool _has_allgathered;
};

template <typename T>
StochasticReporterContext<T>::StochasticReporterContext(const libMesh::ParallelObject & other,
                                                        const MooseObject & producer,
                                                        ReporterState<std::vector<T>> & state,
                                                        const Sampler & sampler)
  : ReporterGeneralContext<std::vector<T>>(other, producer, state),
    _sampler(sampler),
    _has_gathered(false),
    _has_allgathered(false)
{
  this->_state.value().resize(_sampler.getNumberOfLocalRows());
}

template <typename T>
void
StochasticReporterContext<T>::copyValuesBack()
{
  this->_state.copyValuesBack();
  if (_has_allgathered || (_has_gathered && this->processor_id() == 0))
  {
    auto & val = this->_state.value();
    val.erase(val.begin(), val.begin() + _sampler.getLocalRowBegin());
    val.erase(val.begin() + _sampler.getLocalRowEnd(), val.end());
  }
  _has_gathered = false;
  _has_allgathered = false;
}

template <typename T>
void
StochasticReporterContext<T>::finalize()
{
  bool gather_required = this->_producer_enum == REPORTER_MODE_ROOT;
  bool allgather_required = this->_producer_enum == REPORTER_MODE_REPLICATED;
  for (const auto & pair : this->_state.getConsumers())
  {
    const ReporterMode consumer = pair.first;
    if (consumer == REPORTER_MODE_ROOT)
      gather_required = true;
    else if (consumer == REPORTER_MODE_REPLICATED)
      allgather_required = true;
  }

  if (allgather_required && !_has_allgathered)
    StochasticTools::stochasticAllGather(this->comm(), this->_state.value());
  else if (gather_required && !_has_gathered)
    StochasticTools::stochasticGather(this->comm(), 0, this->_state.value());

  _has_gathered = gather_required || _has_gathered;
  _has_allgathered = allgather_required || _has_allgathered;
}

template <typename T>
void
StochasticReporterContext<T>::storeInfo(nlohmann::json & json) const
{
  ReporterGeneralContext<std::vector<T>>::storeInfo(json);
  if (_has_allgathered || (_has_gathered && this->processor_id() == 0))
  {
    json["row_begin"] = 0;
    json["row_end"] = this->_sampler.getNumberOfRows();
  }
  else
  {
    json["row_begin"] = this->_sampler.getLocalRowBegin();
    json["row_end"] = this->_sampler.getLocalRowEnd();
  }
}

class StochasticReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  StochasticReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  virtual ReporterName declareStochasticReporterClone(const Sampler & sampler,
                                                      const ReporterData & from_data,
                                                      const ReporterName & from_reporter,
                                                      std::string prefix = "");
  template <typename T>
  std::vector<T> & declareStochasticReporter(std::string value_name, const Sampler & sampler);
  friend class SamplerReporterTransfer;

private:
  const unsigned int _parallel_type;
};

template <typename T>
std::vector<T> &
StochasticReporter::declareStochasticReporter(std::string value_name, const Sampler & sampler)
{
  const ReporterMode mode =
      this->_parallel_type == 0 ? REPORTER_MODE_DISTRIBUTED : REPORTER_MODE_ROOT;
  return this->template declareValueByName<std::vector<T>, StochasticReporterContext<T>>(
      value_name, mode, sampler);
}
