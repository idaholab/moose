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
 * Auxiliary kernel for computing the heat generated from Joule heating
 */
class JouleHeatingHeatGeneratedAux : public AuxKernel
{
public:
  static InputParameters validParams();

  JouleHeatingHeatGeneratedAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableGradient & _grad_elec;
  const MaterialProperty<Real> & _elec_cond;
};
