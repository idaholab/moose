/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef RANDOMINTERFACE_H
#define RANDOMINTERFACE_H

#include "InputParameters.h"
#include "FEProblem.h"
#include "ParallelUniqueId.h"

class RandomInterface;
class Assembly;
class RandomData;
class MooseRandom;

template<>
InputParameters validParams<RandomInterface>();

/**
 * Interface for objects that need parallel consistent random numbers without patterns over
 * the course of multiple runs.
 */
class RandomInterface
{
public:
  RandomInterface(const std::string & name, InputParameters & parameters, FEProblem & problem,
                  THREAD_ID tid, bool is_nodal);

  ~RandomInterface();

  /**
   * This interface should be called from a derived class to enable random number
   * generation in this object.
   */
  void setRandomResetFrequency(ExecFlagType exec_flag);

  /**
   * Returns the next random number (long) from the generator tied to this object (elem/node).
   */
  unsigned long getRandomLong();

  /**
   * Returns the next random number (Real) from the generator tied to this object (elem/node).
   */
  Real getRandomReal();

  /**
   * Get the seed for the passed in elem/node id.
   * @param id - dof object id
   * @return current seed for this id
   */
  unsigned int getSeed(unsigned int id);

  /**************************************************
   *                Data Accessors                  *
   **************************************************/
  unsigned int getMasterSeed() const { return _master_seed; }
  bool isNodal() const { return _is_nodal; }
  ExecFlagType getResetOnTime() const { return _reset_on; }

  void setRandomDataPointer(RandomData *random_data);

private:
  RandomData *_random_data;
  MooseRandom *_generator;

  FEProblem & _ri_problem;
  const std::string & _ri_name;

  unsigned int _master_seed;
  bool _is_nodal;
  ExecFlagType _reset_on;

  const Node * & _curr_node;
  const Elem * & _curr_element;

//  friend void FEProblem::registerRandomInterface(RandomInterface *random_interface, const std::string & name, ExecFlagType exec_flag);
};

#endif /* RANDOMINTERFACE_H */
