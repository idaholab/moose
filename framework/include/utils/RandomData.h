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
#include "MooseRandom.h"
#include "MooseTypes.h"
#include "MooseEnumItem.h"

#include <unordered_map>

class FEProblemBase;
class MooseMesh;
class RandomInterface;

class RandomData
{
public:
  RandomData(FEProblemBase & fe_problem, const RandomInterface & random_interface);

  RandomData(FEProblemBase & fe_problem, bool is_nodal, ExecFlagType reset_on, unsigned int seed);

  ~RandomData() = default;

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
