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

class StatisticsVectorPostprocessor;

template <>
InputParameters validParams<StatisticsVectorPostprocessor>();

/**
 * Compute several metrics for each MPI process.
 *
 * Note: this is somewhat expensive.  It does loops over elements, sides and nodes
 */
class StatisticsVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  StatisticsVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /**
   * Compute the passed in statistic for the vector
   */
  Real computeStatValue(int stat_id, const std::vector<Real> & stat_vector);

  /// The name of the VPP to work on
  const VectorPostprocessorName & _vpp_name;

  /// The chosen statistics to compute
  MultiMooseEnum _stats;

  /// The VPP vector that will hold the statistics identifiers
  VectorPostprocessorValue & _stat_type_vector;

  /// The VPP vectors that will hold the statistics for each column
  std::map<std::string, VectorPostprocessorValue *> _stat_vectors;
};

