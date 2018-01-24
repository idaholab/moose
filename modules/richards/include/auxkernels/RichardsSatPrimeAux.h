/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSSATPRIMEAUX_H
#define RICHARDSSATPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsSat.h"

// Forward Declarations
class RichardsSatPrimeAux;

template <>
InputParameters validParams<RichardsSatPrimeAux>();

/**
 * Derivative of fluid Saturation wrt effective saturation
 */
class RichardsSatPrimeAux : public AuxKernel
{
public:
  RichardsSatPrimeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  const VariableValue & _seff_var;

  /// User object defining saturation as a function of effective saturation
  const RichardsSat & _sat_UO;
};

#endif // RICHARDSSATPRIMEAUX_H
