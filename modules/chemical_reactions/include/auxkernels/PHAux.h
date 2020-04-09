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
 * The pH of the solution is defined as
 *
 * pH = - log10(a_H),
 *
 * where a_H is the activity of the H+ ions, defined as
 *
 * a_H = gamma C_H,
 *
 * where gamma is the activity coefficient, and C_H is the molar
 * concentration of free H+ ions in solution
 */
class PHAux : public AuxKernel
{
public:
  static InputParameters validParams();

  PHAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Free molar concentration of H+ ions
  const VariableValue & _hplus;
  /// Activity coefficient of H+ ions
  const VariableValue & _gamma;
};
