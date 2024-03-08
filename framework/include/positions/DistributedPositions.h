//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Positions.h"

/**
 * Positions created by distributing Positions objects onto one another. The first positions object
 * creates the base positions onto which the later ones are translated from.
 */
class DistributedPositions : public Positions
{
public:
  static InputParameters validParams();
  DistributedPositions(const InputParameters & parameters);

  virtual void initialize() override;

private:
  /// Pointers to positions objects that will be distributed
  std::vector<const Positions *> _positions_objs;
};
