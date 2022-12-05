//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Standard library
#include <vector>
#include <memory>

// MOOSE includes
#include "MooseTypes.h"

// Forward Declarations
class FEProblemBase;
class OptimizationFunction;
class InputParameters;

class OptimizationFunctionInnerProductHelper
{
public:
  static InputParameters validParams();
  OptimizationFunctionInnerProductHelper(const InputParameters & parameters);

protected:
  /**
   * This function sets up member variables for the inner product accumulation at certain time.
   * The time step size is needed in order to calculate the actual simulation time (for adjoint
   * calculations)
   *
   * @param time Current simulation time, the actual time is computed via _reverse_time_end
   * @param dt The current time step size
   */
  void setCurrentTime(Real time, Real dt);

  /**
   * Accumulates integration for inner product by multiplying the given value
   * by the function's parameterGradient
   *
   * @param q_point The quadrature point location
   * @param q_inner_product The inner product value for the current quadrature point,
   *                        which is multiplied by the function parameter gradient.
   */
  void update(const Point & q_point, Real q_inner_product);

  /**
   * Accumulates inner product integration in _curr_time_ip vector from another object.
   * This is used for thread joining.
   */
  void add(const OptimizationFunctionInnerProductHelper & other);

  /**
   * Gathers _curr_time_ip from other processors and performs time integration
   * @param result Return vector of inner products
   */
  void getVector(std::vector<Real> & result);

private:
  /// FEProblem used for getting system quantities
  FEProblemBase & _ip_problem;

  /// Function used in optimization
  const OptimizationFunction * const _function;

  /// The final time when we want to reverse the time index in function evaluation
  const Real & _reverse_time_end;
  /// Time the simulation is at
  Real _simulation_time;
  /// Time the actual problem is at, defined by _reverse_time_end
  Real _actual_time;

  /// Vector holding data for each time
  std::vector<std::pair<Real, std::vector<Real>>> _time_ip;
  /// Vector for current time
  std::vector<Real> * _curr_time_ip = nullptr;
};
