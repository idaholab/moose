//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose
#include "GeneralUserObject.h"
#include "MooseMesh.h"

class RandomHitUserObject;

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritance tree*/
class RandomHitSolutionModifier : public GeneralUserObject
{
public:
  static InputParameters validParams();

  RandomHitSolutionModifier(const InputParameters & parameters);

  virtual ~RandomHitSolutionModifier() {}

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() {}

  /**
   * Compute the hit positions for this timestep
   */
  virtual void execute();

  /**
   * Finalize.  This is called _after_ execute() and _after_ threadJoin()!  This is probably where
   * you want to do MPI communication!
   */
  virtual void finalize() {}

protected:
  const RandomHitUserObject & _random_hits;

  std::vector<unsigned int> _nodes_that_were_hit;

  MooseMesh & _mesh;

  MooseVariable & _variable;

  Real _amount;
};
