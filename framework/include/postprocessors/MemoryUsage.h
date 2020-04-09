//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "MemoryUsageReporter.h"
#include "MemoryUtils.h"

/**
 * Output maximum, average, or total process memory usage
 */
class MemoryUsage : public GeneralPostprocessor, public MemoryUsageReporter
{
public:
  static InputParameters validParams();

  MemoryUsage(const InputParameters & parameters);

  virtual void timestepSetup() override;

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override;
  virtual PostprocessorValue getValue() override;

protected:
  enum class MemType
  {
    physical_memory,
    virtual_memory,
    page_faults
  } _mem_type;

  enum class ValueType
  {
    total,
    average,
    max_process,
    min_process
  } _value_type;

  /// The unit prefix for the reported memory statistics (kilobyte, megabyte, etc).
  MemoryUtils::MemUnits _mem_units;

  /// memory usage metric in bytes
  Real _value;

  /// peak memory usage metric in bytes (of multiple samples in the current time step)
  Real _peak_value;

  /// report peak value for multiple samples in a time step
  const bool _report_peak_value;
};
