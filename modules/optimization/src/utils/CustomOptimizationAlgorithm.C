//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CustomOptimizationAlgorithm.h"

// MOOSE includes
#include "MooseUtils.h"

CustomOptimizationAlgorithm::CustomOptimizationAlgorithm()
  : _max_its(100), _it_counter(0), _random_seed(0)
{
  MooseRandom::seed(_random_seed);
}

void
CustomOptimizationAlgorithm::setInitialSolution(const std::vector<Real> & real_sol,
                                                const std::vector<int> & int_sol)
{
  _current_real_solution = real_sol;
  _current_int_solution = int_sol;
  _real_size = _current_real_solution.size();
  _int_size = _current_int_solution.size();
  _size = _real_size + _int_size;
}

void
CustomOptimizationAlgorithm::setSeed(unsigned int seed)
{
  _random_seed = seed;
  MooseRandom::seed(_random_seed);
}
