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

/**
 * Computes the magnitude of a vector whose components are given by up
 * to three coupled variables.
 */
class VectorMagnitudeAux : public AuxKernel
{
public:
  static InputParameters validParams();

  VectorMagnitudeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _x;
  const VariableValue & _y;
  const VariableValue & _z;
};
