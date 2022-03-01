//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseEnumItem.h"

// Forward declarations
class Assembly;
class FEProblemBase;
class InputParameters;
class MooseRandom;
class RandomData;
template <typename T>
InputParameters validParams();

/**
 * Interface for objects that need parallel consistent random numbers without patterns over
 * the course of multiple runs.
 */
class RandomInterface
{
public:
  RandomInterface(const InputParameters & parameters,
                  FEProblemBase & problem,
                  THREAD_ID tid,
                  bool is_nodal);

  ~RandomInterface();

  static InputParameters validParams();

  /**
   * This interface should be called from a derived class to enable random number
   * generation in this object.
   */
  void setRandomResetFrequency(ExecFlagType exec_flag);

  /**
   * Returns the next random number (long) from the generator tied to this object (elem/node).
   */
  unsigned long getRandomLong() const;

  /**
   * Returns the next random number (Real) from the generator tied to this object (elem/node).
   */
  Real getRandomReal() const;

  /**
   * Get the seed for the passed in elem/node id.
   * @param id - dof object id
   * @return current seed for this id
   */
  unsigned int getSeed(std::size_t id);

  /**************************************************
   *                Data Accessors                  *
   **************************************************/
  unsigned int getMasterSeed() const { return _master_seed; }
  bool isNodal() const { return _is_nodal; }
  ExecFlagType getResetOnTime() const { return _reset_on; }

  void setRandomDataPointer(RandomData * random_data);

private:
  RandomData * _random_data;
  mutable MooseRandom * _generator;

  FEProblemBase & _ri_problem;
  const std::string _ri_name;

  unsigned int _master_seed;
  bool _is_nodal;
  ExecFlagType _reset_on;

  const Node * const & _curr_node;
  const Elem * const & _curr_element;

  //  friend void FEProblemBase::registerRandomInterface(RandomInterface *random_interface, const
  //  std::string & name, ExecFlagType exec_flag);
};
