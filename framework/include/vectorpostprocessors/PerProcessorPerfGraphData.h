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

class PerProcessorPerfGraphData;

template <>
InputParameters validParams<PerProcessorPerfGraphData>();

/**
 * Compute several metrics for each MPI process.
 *
 * Note: this is somewhat expensive.  It does loops over elements, sides and nodes
 */
class PerProcessorPerfGraphData : public GeneralVectorPostprocessor
{
public:
  PerProcessorPerfGraphData(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  processor_id_type _my_pid;

  const RankMap & _rank_map;

  unsigned int _my_hardware_id;

  std::vector<int> _data_types;

  const std::vector<std::string> & _section_names;

  VectorPostprocessorValue & _pid;
  VectorPostprocessorValue & _hardware_id;

  std::vector<Real> _local_data;
  std::vector<VectorPostprocessorValue *> _data_vectors;
};
