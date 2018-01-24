/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSDENSITYPRIMEPRIMEAUX_H
#define RICHARDSDENSITYPRIMEPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsDensity.h"

// Forward Declarations
class RichardsDensityPrimePrimeAux;

template <>
InputParameters validParams<RichardsDensityPrimePrimeAux>();

/**
 * Second derivative of fluid density wrt porepressure
 */
class RichardsDensityPrimePrimeAux : public AuxKernel
{
public:
  RichardsDensityPrimePrimeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// porepressure
  const VariableValue & _pressure_var;

  /// userobject that defines density as a fcn of porepressure
  const RichardsDensity & _density_UO;
};

#endif // RICHARDSDENSITYPRIMEPRIMEAUX_H
