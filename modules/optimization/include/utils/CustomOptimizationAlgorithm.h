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
#include "MooseTypes.h"
#include "MooseRandom.h"

class CustomOptimizationAlgorithm
{
public:
  /**
   * Constructor.
   */
  CustomOptimizationAlgorithm();
  virtual ~CustomOptimizationAlgorithm() = default;

  /// sets the objective routine
  void setObjectiveRoutine(void (*fncptr)(Real & objective,
                                          const std::vector<Real> & rparams,
                                          const std::vector<int> & iparams,
                                          void * ctx),
                           void * ctx)
  {
    _ctx = ctx;
    objectiveFunction = fncptr;
  };

  /// purely virtual optimize function
  virtual void solve() = 0;

  ///@{ public interface
  unsigned int & maxIt() { return _max_its; }
  void setInitialSolution(const std::vector<Real> & real_sol, const std::vector<int> & int_sol);
  void setSeed(unsigned int seed);
  const std::vector<Real> & realSolution() const { return _current_real_solution; }
  const std::vector<int> & intSolution() const { return _current_int_solution; }
  ///@}

protected:
  void (*objectiveFunction)(Real & objective,
                            const std::vector<Real> & rparams,
                            const std::vector<int> & iparams,
                            void * ctx);
  void * _ctx;

  /// maximum number of steps/iterations
  unsigned int _max_its;

  /// iteration counter
  unsigned int _it_counter;

  /// a random seed in case randomization is used for optimization algorithm
  unsigned int _random_seed;

  ///@{ the current solution
  std::vector<Real> _current_real_solution;
  std::vector<int> _current_int_solution;
  unsigned int _real_size;
  unsigned int _int_size;
  unsigned int _size;
  ///@}
};
