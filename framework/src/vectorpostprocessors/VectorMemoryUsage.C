//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorMemoryUsage.h"
#include "MemoryUtils.h"
#include <algorithm>

#include "Conversion.h"

registerMooseObject("MooseApp", VectorMemoryUsage);

InputParameters
VectorMemoryUsage::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Get memory stats for all ranks in the simulation");
  params.addParam<bool>("report_peak_value",
                        true,
                        "If the vectorpostprocessor is executed more than once "
                        "during a time step, report the aggregated peak "
                        "value.");
  params.addParam<MooseEnum>("mem_units",
                             MemoryUtils::getMemUnitsEnum(),
                             "The unit prefix used to report memory usage, default: Mebibytes");
  return params;
}

VectorMemoryUsage::VectorMemoryUsage(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    MemoryUsageReporter(this),
    _mem_units(getParam<MooseEnum>("mem_units").getEnum<MemoryUtils::MemUnits>()),
    _col_hardware_id(declareVector("hardware_id")),
    _col_total_ram(declareVector("total_ram")),
    _col_physical_mem(declareVector("physical_mem")),
    _col_virtual_mem(declareVector("virtual_mem")),
    _col_page_faults(declareVector("page_faults")),
    _col_node_utilization(declareVector("node_utilization")),
    _report_peak_value(getParam<bool>("report_peak_value")),
    _peak_physical_mem(0.0),
    _peak_virtual_mem(0.0)
{
  _col_hardware_id.resize(_nrank);
  _col_total_ram.resize(_nrank);
  _col_physical_mem.resize(_nrank);
  _col_virtual_mem.resize(_nrank);
  _col_page_faults.resize(_nrank);
  _col_node_utilization.resize(_nrank);

  if (_my_rank == 0)
  {
    std::copy(_hardware_id.begin(), _hardware_id.end(), _col_hardware_id.begin());
    for (std::size_t i = 0; i < _nrank; ++i)
      _col_total_ram[i] =
          MemoryUtils::convertBytes(_hardware_memory_total[_hardware_id[i]], _mem_units);
  }
}

void
VectorMemoryUsage::timestepSetup()
{
  _peak_physical_mem = 0.0;
  _peak_virtual_mem = 0.0;
}

void
VectorMemoryUsage::execute()
{
  // skip during checking uo/aux state so that this postprocessor returns the memory
  // usage during the regular execution
  if (_fe_problem.checkingUOAuxState())
    return;

  MemoryUtils::Stats stats;
  MemoryUtils::getMemoryStats(stats);

  if (_report_peak_value)
  {
    _peak_physical_mem = std::max(_peak_physical_mem, static_cast<Real>(stats._physical_memory));
    _peak_virtual_mem = std::max(_peak_virtual_mem, static_cast<Real>(stats._virtual_memory));
  }

  _col_physical_mem[_my_rank] = _report_peak_value ? _peak_physical_mem : stats._physical_memory;
  _col_virtual_mem[_my_rank] = _report_peak_value ? _peak_virtual_mem : stats._virtual_memory;
  _col_page_faults[_my_rank] = stats._page_faults;
}

void
VectorMemoryUsage::finalize()
{
  // skip during checking uo/aux state so that this postprocessor returns the memory
  // usage during the regular execution
  if (_fe_problem.checkingUOAuxState())
    return;

  // send data to rank zero (avoid buffer aliasing error using out-of-vector copies)
  auto local_physical_mem = _col_physical_mem[_my_rank];
  _communicator.gather(0, local_physical_mem, _col_physical_mem);

  auto local_virtual_mem = _col_virtual_mem[_my_rank];
  _communicator.gather(0, local_virtual_mem, _col_virtual_mem);

#ifndef __APPLE__
  auto local_page_faults = _col_page_faults[_my_rank];
  _communicator.gather(0, local_page_faults, _col_page_faults);
#endif

  // prepare node utilization column
  if (_my_rank == 0)
  {
    std::vector<Real> physical_per_node(_hardware_memory_total.size());
    for (std::size_t i = 0; i < _nrank; ++i)
      physical_per_node[_hardware_id[i]] += _col_physical_mem[i];

    for (std::size_t i = 0; i < _nrank; ++i)
      _col_node_utilization[i] = physical_per_node[_hardware_id[i]] /
                                 static_cast<Real>(_hardware_memory_total[_hardware_id[i]]);
  }

  // unit prefix conversion
  for (std::size_t i = 0; i < _nrank; ++i)
  {
    _col_physical_mem[i] = MemoryUtils::convertBytes(_col_physical_mem[i], _mem_units);
    _col_virtual_mem[i] = MemoryUtils::convertBytes(_col_virtual_mem[i], _mem_units);
  }
}
