//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

class SolverSystem;
class Convergence;

/**
 * A solve object for use when wanting to solve multiple systems
 */
class MultiSystemSolveObject : public SolveObject
{
public:
  MultiSystemSolveObject(Executioner & ex);

  static InputParameters validParams();

  /**
   * Returns a reference to the vector of solver systems that this object is
   * supposed to solve
   */
  const std::vector<SolverSystem *> & systemsToSolve() const { return _systems; }

protected:
  /// Vector of pointers to the systems
  std::vector<SolverSystem *> _systems;

  /// Number of nonlinear systems
  unsigned int _num_nl_systems;

  /// Whether we are using fixed point iterations for multi-system
  const bool _using_multi_sys_fp_iterations;
  /// Convergence object to assess the convergence of the multi-system fixed point iteration
  Convergence * _multi_sys_fp_convergence;
  /// Per-system relaxation factors for multi-system fixed point iterations (expanded to
  /// match the number/order of systems being solved)
  std::vector<Real> _multi_sys_fp_relax_factors;

private:
  /// Initializes/expands the multi-system fixed point relaxation factors
  void setupMultiSystemFixedPointRelaxationFactors();
};
