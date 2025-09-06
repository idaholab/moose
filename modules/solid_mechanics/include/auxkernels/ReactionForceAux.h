//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TagVectorAux.h"

/**
 * ReactionForceAux returns the reaction force corresponding to each DOF.
 */
class ReactionForceAux : public TagVectorAux
{
public:
  static InputParameters validParams();

  ReactionForceAux(const InputParameters & parameters);
};
