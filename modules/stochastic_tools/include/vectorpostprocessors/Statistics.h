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
#include "BootstrapCalculators.h"

/**
 * Compute several metrics for supplied VPP vectors.
 *
 * This class uses calculator objects defined in Statistics.h and is setup such that if a new
 * calculation is needed it can be added in Statistics.h without modification of this object.
 */
class Statistics : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();
  Statistics(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void execute() override;

  /// Not used; all parallel computation is wrapped in the Statistics objects
  void initialize() override final;
  void finalize() override final {}

protected:
  /// The selected statistics to compute
  const MultiMooseEnum & _compute_stats;

  /// Bootstrap Confidence Level method
  const MooseEnum & _ci_method;

  /// Confidence levels to compute (see computeLevels)
  const std::vector<Real> _ci_levels;

  /// Confidence level replicates
  const unsigned int _replicates;

  /// Confidence level seed
  const unsigned int _seed;

  /// The VPP vector that will hold the statistics identifiers
  VectorPostprocessorValue & _stat_type_vector;

  // The following vectors are sized to the number of statistics to be computed

  /// The VPP vectors being computed
  std::vector<VectorPostprocessorValue *> _stat_vectors;

  /// VPPs names to be computed from (Vectorpostprocessor name, vector name, is_distribute)
  std::vector<std::tuple<std::string, std::string, bool>> _compute_from_names;

private:
  /**
   * Helper function for converting confidence levels given in (0, 0.5] into levels in (0, 1).
   * For example, levels_in = {0.05, 0.1, 0.5} converts to {0.05 0.1, 0.5, 0.9, 0.95}.
   *
   * This also performs error checking on the supplied "ci_levels".
   */
  std::vector<Real> computeLevels(const std::vector<Real> & levels_in) const;
};
