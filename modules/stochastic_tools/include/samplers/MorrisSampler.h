//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * A class used to perform Monte Carlo sampling for performing Morris sensitivity analysis.
 *
 */
class MorrisSampler : public Sampler
{
public:
  static InputParameters validParams();

  MorrisSampler(const InputParameters & parameters);

protected:
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /**
   * Morris sampling should have a slightly different partitioning in order to keep
   * the sample and resample samplers distributed and make computing indices more
   * efficient.
   */
  virtual LocalRankConfig constructRankConfig(bool batch_mode) const override;

  /// Number of trajectories
  const dof_id_type & _num_trajectories;
  /// Number of levels used for input space discretization
  const unsigned int & _num_levels;
  /// Distribution to determine parameter from perturbations
  std::vector<const Distribution *> _distributions;

private:
  /**
   * Function to calculate trajectories
   * This is only called once per trajectory (_n_rows / (_n_cols + 1))
   */
  void updateBstar();

  /**
   * Stateless RNG variant of updateBstar().
   * Computes the same trajectory data using stateless random draws.
   */
  void updateBstarStateless();

  /// The trajectory the current _bstar represents
  dof_id_type _curr_trajectory = std::numeric_limits<dof_id_type>::max();

  ///@{
  /// Matrices used for trajectory computation
  RealEigenMatrix _b;
  RealEigenMatrix _pstar;
  RealEigenMatrix _j;
  RealEigenMatrix _dstar;
  RealEigenMatrix _xstar;
  RealEigenMatrix _bstar;
  ///@}
};
