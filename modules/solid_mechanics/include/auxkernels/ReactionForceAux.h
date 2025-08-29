//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TagResidualAux.h"

/**
 * Similar to TagResidualAux, but returns the _unscaled_ vector value that is consistent with the
 * units of the simulation.
 */
class ReactionForceAux : public TagResidualAux
{
public:
  static InputParameters validParams();

  ReactionForceAux(const InputParameters & parameters);
};
