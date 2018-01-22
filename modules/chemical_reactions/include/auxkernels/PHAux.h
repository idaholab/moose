/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PHAUX_H
#define PHAUX_H

#include "AuxKernel.h"

class PHAux;

template <>
InputParameters validParams<PHAux>();

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
  PHAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Free molar concentration of H+ ions
  const VariableValue & _hplus;
  /// Activity coefficient of H+ ions
  const VariableValue & _gamma;
};

#endif // PHAUX_H
