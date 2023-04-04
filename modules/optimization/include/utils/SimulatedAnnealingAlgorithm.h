//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "CustomOptimizationAlgorithm.h"

class SimulatedAnnealingAlgorithm : public CustomOptimizationAlgorithm
{
public:
  /**
   * Constructor.
   */
  SimulatedAnnealingAlgorithm();

  /// cooling enum
  enum Cooling
  {
    LinMult,
    ExpMult,
    LogMult,
    QuadMult,
    LinAdd,
    QuadAdd,
    ExpAdd,
    TrigAdd
  };

  enum RealNeighborSelection
  {
    RandomDirectionStretching,
    BoxSampling
  };

  /// purely virtual optimize function
  void solve() override;

  ///@{ public interface
  Cooling & cooling() { return _cooling; }
  unsigned int & numSwaps() { return _num_swaps; }
  unsigned int & numReassignments() { return _num_reassignments; }
  void setValidReassignmentOptions(const std::set<int> & options);
  Real & relativePerturbationSize() { return _relative_perturbation; }
  void setLowerLimits(const std::vector<Real> & lower_limits);
  void setUpperLimits(const std::vector<Real> & upper_limits);
  RealNeighborSelection & realNeighborSelection() { return _real_perturbation_type; }
  ///@}

protected:
  /// cooling schedule
  Real coolingSchedule(unsigned int step) const;

  /// creates neighbor states from current_states for the continuous params
  void createNeigborReal(const std::vector<Real> & real_sol, std::vector<Real> & real_neigh) const;

  /// creates neighbor states from current_states for the continuous params
  void createNeigborInt(const std::vector<int> & int_sol, std::vector<int> & int_neigh) const;

  /// computes the probability with which a neigbor objective value will be accepted given current objective value and temperature
  Real acceptProbability(Real curr_obj, Real neigh_obj, Real curr_temp) const;

  /// random direction on the unit sphere
  void randomDirection(unsigned int size, std::vector<Real> & direction) const;

  /// state size of the integer space
  unsigned int _int_state_size;

  /// state size of the real space
  unsigned int _real_state_size;

  /// alpha value for cooling
  Real _alpha;

  /// maximum temperature
  Real _temp_max;

  /// minimum temperature
  Real _temp_min;

  /// the best (aka min) objective seen so far
  Real _min_objective;

  /// cooling option
  Cooling _cooling;

  /// if cooling is monotonic or not
  bool _monotonic_cooling;

  /**
   * the temperature where simulated annealing starts resetting the current state
   * to the best found state. This temperature is halved every-time it is reached.
   */
  Real _res_var;

  ///@{ parameters governing the creation of neighbors for int params
  unsigned int _num_swaps;
  unsigned int _num_reassignments;
  std::vector<int> _valid_options;
  ///@}

  ///@{ parameters governing the creation of neighbors for real params
  RealNeighborSelection _real_perturbation_type;
  Real _relative_perturbation;
  bool _upper_limit_provided;
  bool _lower_limit_provided;
  std::vector<Real> _parameter_lower_limit;
  std::vector<Real> _parameter_upper_limit;
  ///@}
};
