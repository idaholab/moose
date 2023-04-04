//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimulatedAnnealingAlgorithm.h"
#include <limits>

SimulatedAnnealingAlgorithm::SimulatedAnnealingAlgorithm()
  : CustomOptimizationAlgorithm(),
    _alpha(1e-2),
    _temp_max(100.0),
    _temp_min(0.0),
    _min_objective(std::numeric_limits<Real>::max()),
    _cooling(LinAdd),
    _monotonic_cooling(true),
    _res_var(0.0),
    _num_swaps(1),
    _num_reassignments(0)
{
}

void
SimulatedAnnealingAlgorithm::solve()
{
  // check that solution size has been set
  if (_size == 0)
    ::mooseError("Solution size is zero. Most likely setInitialSolution was not called.");

  // check neighbor generation options for int params
  if (_int_state_size > 0 && _num_reassignments > 0 && _valid_options.size() < 2)
    ::mooseError("If the number of reassignments for neigbor generation is > 0, then the number of "
                 "valid options must be at least 2.");

  if (_int_state_size > 0 && _num_swaps + _num_reassignments == 0)
    ::mooseError("The problem has a non-zero number of categorical parameters, but the number of "
                 "swaps and number of reassignments for neighbor generation are both 0.");

  // set neighbor & best states
  std::vector<Real> neighbor_real_solution = _current_real_solution;
  std::vector<int> neighbor_int_solution = _current_int_solution;
  std::vector<Real> best_real_solution = _current_real_solution;
  std::vector<int> best_int_solution = _current_int_solution;
  Real current_objective;
  objectiveFunction(current_objective, _current_real_solution, _current_int_solution, _ctx);
  _min_objective = current_objective;

  Real temp_current = _temp_max;

  // the number of "accepted" steps [not necessarily equal to # of traversals of while loop]
  _it_counter = 0;

  // simulated annealing loop
  while (_it_counter < _max_its && temp_current > _temp_min)
  {
    // get a new neighbor and compute energy
    createNeigborReal(_current_real_solution, neighbor_real_solution);
    createNeigborInt(_current_int_solution, neighbor_int_solution);

    Real neigh_objective;
    objectiveFunction(neigh_objective, neighbor_real_solution, neighbor_int_solution, _ctx);

    // acceptance check: lower objective always accepted;
    // higher objective sometimes accepted
    Real temp_r = MooseRandom::rand();
    if (temp_r <= accept_prob(current_objective, neigh_objective, temp_current))
    {
      // if we accept then it always counts as a new step
      ++_it_counter;
      _current_real_solution = neighbor_real_solution;
      _current_int_solution = neighbor_int_solution;
      current_objective = neigh_objective;
    }
    else
    {
      // otherwise, it has a 50% chance to count as a new step to finish the problem
      // this is especially important for combinatorial problems
      Real temp_rr = MooseRandom::rand();
      if (temp_rr <= 0.5)
        ++_it_counter;
    }

    // cool the temperature
    temp_current = coolingSchedule(_it_counter);

    // if this is the best energy, it's our new best value
    if (current_objective < _min_objective)
    {
      _min_objective = current_objective;
      best_real_solution = _current_real_solution;
      best_int_solution = _current_int_solution;
    }

    // perform non-monotonic adjustment if applicable
    if (!_monotonic_cooling)
      temp_current *= (1.0 + (current_objective - _min_objective) / current_objective);

    // rewind to best value if reset is enabled
    if (std::abs(temp_current) <= _res_var)
    {
      _res_var *= 0.5;
      current_objective = _min_objective;
      _current_real_solution = best_real_solution;
      _current_int_solution = best_int_solution;
    }
  }

  // select the best state we ended up finding
  current_objective = _min_objective;
  _current_real_solution = best_real_solution;
  _current_int_solution = best_int_solution;
}

Real
SimulatedAnnealingAlgorithm::coolingSchedule(unsigned int step) const
{
  switch (_cooling)
  {
    case LinAdd:
      return _temp_min + (_temp_max - _temp_min) * ((Real)_max_its - (Real)step) / (Real)_max_its;
    default:
      ::mooseError("Cooling option not yet implemented");
  }

  return 1;
}

void
SimulatedAnnealingAlgorithm::createNeigborReal(const std::vector<Real> & real_sol,
                                               std::vector<Real> & real_neigh) const
{
  //
  if (_real_size == 0)
  {
    real_neigh = {};
    return;
  }

  // set neighbor to the current state
  real_neigh = real_sol;

  // we use
}

void
SimulatedAnnealingAlgorithm::createNeigborInt(const std::vector<int> & int_sol,
                                              std::vector<int> & int_neigh) const
{
  if (_int_size == 0)
  {
    int_neigh = {};
    return;
  }

  // set neighbor to the current state
  int_neigh = int_sol;

  // loop that ensures that there is a difference in int_sol and int_neigh
  int diff = 0;
  while (diff < 1)
  {
    // swaps
    for (unsigned int j = 0; j < _num_swaps; ++j)
    {
      auto j1 = MooseRandom::randl() % _int_size;
      auto j2 = MooseRandom::randl() % _int_size;
      mooseAssert(j1 < _int_size, "The index needs to be smaller than integer size");
      mooseAssert(j2 < _int_size, "The index needs to be smaller than integer size");
      int_neigh[j1] = int_sol[j2];
      int_neigh[j2] = int_sol[j1];
    }

    // reassignments
    for (unsigned int j = 0; j < _num_reassignments; ++j)
    {
      if (_valid_options.size() == 0)
        mooseError("The number of reassignments is > 0, but no valid options for the "
                   "integer/categorical parameters was provided.");

      auto j1 = MooseRandom::randl() % _int_size;
      auto j2 = MooseRandom::randl() % _valid_options.size();
      mooseAssert(j1 < _int_size, "The index needs to be smaller than integer size");
      mooseAssert(j2 < _valid_options.size(),
                  "The index needs to be smaller than valid options size");
      int_neigh[j1] = _valid_options[j2];
    }

    // compute difference between int_sol and int_neigh
    diff = 0;
    for (unsigned int j = 0; j < _int_size; ++j)
      diff += std::abs(int_neigh[j] - int_sol[j]);
  }
}

Real
SimulatedAnnealingAlgorithm::acceptProbability(Real curr_obj, Real neigh_obj, Real curr_temp) const
{
  Real delta_obj = neigh_obj - curr_obj;
  Real aprob;
  if (-delta_obj / curr_temp <= -700.0)
    aprob = 0.0;
  else if (-delta_obj / curr_temp >= 700.0)
    aprob = 10.0;
  else
    aprob = std::exp(-delta_obj / curr_temp);

  if (delta_obj <= 0.0)
    aprob = 10.0;

  if (std::isnan(aprob))
    aprob = 0.0;
  return aprob;
}

void
SimulatedAnnealingAlgorithm::setValidReassignmentOptions(const std::set<int> & options)
{
  if (options.size() == 0)
    mooseError("Empty set of options");
  _valid_options.clear();
  _valid_options.resize(options.size());
  unsigned int j = 0;
  for (auto & p : options)
    _valid_options[j++] = p;
}
