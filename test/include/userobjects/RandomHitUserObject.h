//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

#include "MooseRandom.h"

class RandomHitUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  RandomHitUserObject(const InputParameters & parameters);

  virtual ~RandomHitUserObject() {}

  /**
   * Returns true if the element was hit.
   * @param elem The element to be checked.
   */
  bool elementWasHit(const Elem * elem) const;

  /**
   * Returns the hits for this timestep.
   */
  const std::vector<Point> & hits() const { return _hit_positions; }

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() {}

  /**
   * Compute the hit positions for this timestep
   */
  virtual void execute();

  virtual void finalize() {}

protected:
  unsigned int _num_hits;
  std::vector<Point> _hit_positions;
  std::vector<unsigned int> _closest_node;
  MooseRandom _random;
};
