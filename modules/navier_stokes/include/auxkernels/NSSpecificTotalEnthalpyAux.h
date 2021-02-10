//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "AuxKernel.h"

// Forward Declarations

/**
 * Nodal auxiliary variable, for computing enthalpy at the nodes.
 * The total enthalpy is given by:
 *
 * H = E + p/rho (primitive variables)
 * H = (U_4 + P(U)) / U_0 (conserved variables)
 *
 * where P(U) = (gamma-1)*(U_4 - (1/2)*(U_1^2 + U_2^2 + U_3^2)/U_0)
 * is the pressure.
 */
class NSSpecificTotalEnthalpyAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NSSpecificTotalEnthalpyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _rho_et;
  const VariableValue & _pressure;
};
