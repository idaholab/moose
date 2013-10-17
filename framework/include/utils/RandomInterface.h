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
#include "MooseRandom.h"
#include "ParallelUniqueId.h"

#include "libmesh/libmesh_config.h"
#include LIBMESH_INCLUDE_UNORDERED_MAP

class RandomInterface;
class FEProblem;
class Assembly;
class MooseMesh;

template<>
InputParameters validParams<RandomInterface>();

/**
 * Interface for objects that need parallel consistent random numbers without patterns over
 * the course of multiple runs.
 */
class RandomInterface
{
public:
  RandomInterface(InputParameters & parameters, FEProblem & problem, THREAD_ID tid, bool is_nodal);
  ~RandomInterface();

  void setRandomResetFrequency(ExecFlagType exec_flag);

  virtual void updateSeeds(ExecFlagType exec_flag);


  unsigned long getRandomLong();

  Real getRandomReal();


  void updateMasterSeed(unsigned int seed);

  unsigned int getSeed(unsigned int id);

private:
  MooseRandom _generator;
  FEProblem & _ri_problem;
  Assembly & _ri_assembly;
  MooseMesh & _ri_mesh;
  THREAD_ID _ri_tid;
  unsigned int _master_seed;
  bool _is_nodal;

  unsigned int _current_master_seed;
  ExecFlagType _reset_on;

  const Node * & _curr_node;
  const Elem * & _curr_element;

  // dof id to seed
  LIBMESH_BEST_UNORDERED_MAP<dof_id_type, unsigned int> _seeds;
};

#endif /* RANDOMINTERFACE_H */
