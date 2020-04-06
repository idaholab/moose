//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "RandomData.h"
#include "MooseRandom.h"

// Forward Declarations
class InputParameters;
namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

/**
 * Base class for randomly generated initial conditions
 */
class RandomICBase : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  RandomICBase(const InputParameters & parameters);

  void initialSetup() override;

  static InputParameters validParams();

protected:
  /**
   * Generate a uniformly distributed random number on the interval from 0 to 1
   * @return random number
   */
  Real generateRandom();

  /// Determines whether a variable basis is elemental or nodal
  const bool _is_nodal;

  /// Boolean to indicate whether we want to use the old (deprecated) generation pattern
  const bool _use_legacy;

private:
  /// RandomData element object, we cannot inherit from RandomInterface in an InitialCondition
  std::unique_ptr<RandomData> _elem_random_data;

  /// RandomData node object, we cannot inherit from RandomInterface in an InitialCondition
  std::unique_ptr<RandomData> _node_random_data;

  /// Elemental random number generator
  MooseRandom * _elem_random_generator;

  /// Nodal random number generator
  MooseRandom * _node_random_generator;

  /// Random numbers per element (currently limited to a single value at a time)
  std::map<dof_id_type, Real> _elem_numbers;

  /// Random numbers per node (currently limited to a single value at a time)
  std::map<dof_id_type, Real> _node_numbers;
};
