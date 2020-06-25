//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations
class RandomElementalUserObject;

/**
 * An AuxKernel that uses built-in Random number generation.
 */
class RandomAux : public AuxKernel
{
public:
  static InputParameters validParams();

  RandomAux(const InputParameters & params);

  virtual ~RandomAux();

protected:
  virtual Real computeValue();

  const RandomElementalUserObject * const _random_uo;
  bool _generate_ints;
};
