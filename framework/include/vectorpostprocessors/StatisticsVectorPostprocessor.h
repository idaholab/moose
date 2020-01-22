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
#include "Statistics.h"

class StatisticsVectorPostprocessor;

template <>
InputParameters validParams<StatisticsVectorPostprocessor>();

/**
 * Compute several metrics for supplied VPP vectors.
 *
 * This class uses calculator objects defined in Statistics.h and is setup such that if a new
 * calculation is needed it can be added in Statistics.h without modification of this object.
 */
class StatisticsVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  StatisticsVectorPostprocessor(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// The selected statistics to compute
  const MultiMooseEnum & _compute_stats;

  /// The VPP vector that will hold the statistics identifiers
  VectorPostprocessorValue & _stat_type_vector;

  // The following vectors are sized to the number of statistics to be computed

  /// The VPP vectors being computed
  std::vector<VectorPostprocessorValue *> _stat_vectors;

  /// Calculators, 1 for each vector and each statistic to compute for that vector
  std::vector<std::vector<std::unique_ptr<Statistics::Calculator>>> _stat_calculators;

  /// VPPs names to be computed from
  /// (Vectorpostprocessor name, vector name, is_distribute)
  std::vector<std::tuple<std::string, std::string, bool>> _compute_from_names;
};
