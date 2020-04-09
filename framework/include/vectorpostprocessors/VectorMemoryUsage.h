//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"
#include "MemoryUsageReporter.h"
#include "MemoryUtils.h"

/**
 * Generate a table of various memory metrics indexed by MPI rank. Visualize this using
 * VectorPostprocessorVisualizationAux.
 */
class VectorMemoryUsage : public GeneralVectorPostprocessor, public MemoryUsageReporter
{
public:
  static InputParameters validParams();

  VectorMemoryUsage(const InputParameters & parameters);

  virtual void timestepSetup() override;

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// The unit prefix for the reported memory statistics (kilobyte, megabyte, etc).
  MemoryUtils::MemUnits _mem_units;

  /// hardware id for the physical node the rank is located at
  VectorPostprocessorValue & _col_hardware_id;

  /// total RAM available on the physical node the rank is located at
  VectorPostprocessorValue & _col_total_ram;

  /// physical memory usage per rank
  VectorPostprocessorValue & _col_physical_mem;

  /// virtual memory usage per rank
  VectorPostprocessorValue & _col_virtual_mem;

  /// hard page faults per rank (Linux only), i.e. swap frequency
  VectorPostprocessorValue & _col_page_faults;

  /// RAM utilization of the physical node (i.e. what fraction of the total RAM is the simulation using)
  VectorPostprocessorValue & _col_node_utilization;

  ///@{ peak values
  const bool _report_peak_value;
  Real _peak_physical_mem;
  Real _peak_virtual_mem;
  ///@}
};
