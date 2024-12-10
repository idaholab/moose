//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiffusionFluxAux.h"

/**
 * Computes linear heat rate in a 2D-RZ model of a fuel pin
 */
class SCMRZPinQPrimeAux : public DiffusionFluxAux
{
public:
  static InputParameters validParams();

  SCMRZPinQPrimeAux(const InputParameters & parameters);

  virtual Real computeValue() override;
};
