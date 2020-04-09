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

/**
 * Nodal auxiliary variable for enthalpy,
 *
 * H = E + p/rho
 *
 */
class EnthalpyAux : public AuxKernel
{
public:
  static InputParameters validParams();

  EnthalpyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _rho_et;
  const VariableValue & _pressure;
};
