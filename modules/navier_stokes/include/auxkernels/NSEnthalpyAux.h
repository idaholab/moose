/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENTHALPYAUX_H
#define NSENTHALPYAUX_H

// MOOSE includes
#include "AuxKernel.h"

// Forward Declarations
class NSEnthalpyAux;

template <>
InputParameters validParams<NSEnthalpyAux>();

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
class NSEnthalpyAux : public AuxKernel
{
public:
  NSEnthalpyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _rhoE;
  const VariableValue & _pressure;
};

#endif // NSENTHALPYAUX_H
