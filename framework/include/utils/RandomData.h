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

#ifndef RANDOMDATA_H
#define RANDOMDATA_H

// MOOSE includes
#include "MooseTypes.h"
#include "MooseRandom.h"

#include <unordered_map>

class FEProblemBase;
class MooseMesh;
class RandomInterface;

class RandomData
{
public:
  RandomData(FEProblemBase & problem, const RandomInterface & random_interface);

  /**
   * This method is called to reset or update the seeds based on the reset_on
   * flag and the passed execution point.
   */
  void updateSeeds(ExecFlagType exec_flag);

  /**
   * Return the underlying MooseRandom generator object for this data instance.
   */
  MooseRandom & getGenerator() { return _generator; }

  /**
   * Get the seed for the passed in elem/node id.
   * @param id - dof object id
   * @return current seed for this id
   */
  unsigned int getSeed(dof_id_type id);

private:
  void updateGenerators();

  template <typename T>
  void updateGeneratorHelper(T it, T end_it);

  FEProblemBase & _rd_problem;
  MooseMesh & _rd_mesh;

  MooseRandom _generator;
  bool _is_nodal;
  ExecFlagType _reset_on;

  unsigned int _master_seed;
  unsigned int _current_master_seed;
  unsigned int _new_seed;

  std::unordered_map<dof_id_type, unsigned int> _seeds;
};

#endif // RANDOMDATA_H
