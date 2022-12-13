//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MemoryUsage.h"
#include "MemoryUtils.h"

#include <array>

registerMooseObject("MooseApp", MemoryUsage);

InputParameters
MemoryUsage::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Memory usage statistics for the running simulation.");
  MooseEnum mem_type("physical_memory virtual_memory page_faults", "physical_memory");
  params.addParam<MooseEnum>("mem_type", mem_type, "Memory metric to report.");
  MooseEnum value_type("total average max_process min_process", "total");
  params.addParam<MooseEnum>(
      "value_type", value_type, "Aggregation method to apply to the requested memory metric.");
  params.addParam<MooseEnum>("mem_units",
                             MemoryUtils::getMemUnitsEnum(),
                             "The unit prefix used to report memory usage, default: Mebibytes");
  params.addParam<bool>("report_peak_value",
                        true,
                        "If the postprocessor is executed more than once "
                        "during a time step, report the aggregated peak "
                        "value.");
  return params;
}

MemoryUsage::MemoryUsage(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    MemoryUsageReporter(this),
    _mem_type(getParam<MooseEnum>("mem_type").getEnum<MemType>()),
    _value_type(getParam<MooseEnum>("value_type").getEnum<ValueType>()),
    _mem_units(getParam<MooseEnum>("mem_units").getEnum<MemoryUtils::MemUnits>()),
    _value(0.0),
    _peak_value(0.0),
    _report_peak_value(getParam<bool>("report_peak_value"))
{
}

void
MemoryUsage::timestepSetup()
{
  _peak_value = 0.0;
}

void
MemoryUsage::execute()
{
  // skip during checking uo/aux state so that this postprocessor returns the memory
  // usage during the regular execution
  if (_fe_problem.checkingUOAuxState())
    return;

  MemoryUtils::Stats stats;
  MemoryUtils::getMemoryStats(stats);

  // get the requested per core metric
  switch (_mem_type)
  {
    case MemType::physical_memory:
      _value = MemoryUtils::convertBytes(stats._physical_memory, _mem_units);
      break;

    case MemType::virtual_memory:
      _value = MemoryUtils::convertBytes(stats._virtual_memory, _mem_units);
      break;

    case MemType::page_faults:
      // major page faults are currently only reported on Linux systems
      _value = stats._page_faults;
      break;
  }
}

void
MemoryUsage::finalize()
{
  // skip during checking uo/aux state so that this postprocessor returns the memory
  // usage during the regular execution
  if (_fe_problem.checkingUOAuxState())
    return;

  // perform the requested reduction
  switch (_value_type)
  {
    case ValueType::total:
      gatherSum(_value);
      break;

    case ValueType::average:
      gatherSum(_value);
      _value /= n_processors();
      break;

    case ValueType::max_process:
      gatherMax(_value);
      break;

    case ValueType::min_process:
      gatherMin(_value);
      break;

    default:
      mooseError("Invalid value_type");
  }

  if (_value > _peak_value)
    _peak_value = _value;
}

PostprocessorValue
MemoryUsage::getValue()
{
  return _report_peak_value ? _peak_value : _value;
}
