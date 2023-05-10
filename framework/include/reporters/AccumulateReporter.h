//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

class AccumulatedValueBase;
template <typename T>
class AccumulatedValue;

class AccumulateReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  AccumulateReporter(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /**
   * Helper for declaring an accumulative reporter value
   * This will fill in _accumulated_values if the reporter value is found
   */
  template <typename T>
  bool declareAccumulateHelper(const ReporterName & rname);

  /// Vector of accumulated value objects
  std::vector<std::unique_ptr<AccumulatedValueBase>> _accumulated_values;
};

template <typename T>
bool
AccumulateReporter::declareAccumulateHelper(const ReporterName & rname)
{
  const ReporterData & rdata = _fe_problem.getReporterData();

  if (!rdata.hasReporterValue<T>(rname))
    return false;

  const auto & pmode = rdata.getReporterMode(rname);
  ReporterMode rmode = REPORTER_MODE_UNSET;
  if (pmode == REPORTER_MODE_ROOT)
    rmode = REPORTER_MODE_ROOT;
  else if (pmode == REPORTER_MODE_REPLICATED)
    rmode = REPORTER_MODE_REPLICATED;
  else if (pmode == REPORTER_MODE_DISTRIBUTED)
    rmode = REPORTER_MODE_DISTRIBUTED;
  const T & val = getReporterValueByName<T>(rname);
  std::vector<T> & acc_val =
      declareValueByName<std::vector<T>>(rname.getObjectName() + ":" + rname.getValueName(), rmode);

  _accumulated_values.push_back(std::make_unique<AccumulatedValue<T>>(val, acc_val));
  return true;
}

class AccumulatedValueBase
{
public:
  virtual ~AccumulatedValueBase() = default;

  virtual void accumulate(unsigned int index) = 0;
};

template <typename T>
class AccumulatedValue : public AccumulatedValueBase
{
public:
  AccumulatedValue(const T & val, std::vector<T> & acc_val)
    : AccumulatedValueBase(), _val(val), _acc_val(acc_val)
  {
  }

  virtual void accumulate(unsigned int index) override
  {
    if (_acc_val.size() <= index)
      _acc_val.resize(index + 1, _val);
    else
      _acc_val[index] = _val;
  }

private:
  const T & _val;
  std::vector<T> & _acc_val;
};
