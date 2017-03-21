/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSDENSITYPRIMEAUX_H
#define RICHARDSDENSITYPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsDensity.h"

// Forward Declarations
class RichardsDensityPrimeAux;

template <>
InputParameters validParams<RichardsDensityPrimeAux>();

/**
 * Derivative of fluid density wrt porepressure
 */
class RichardsDensityPrimeAux : public AuxKernel
{
public:
  RichardsDensityPrimeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// porepressure
  const VariableValue & _pressure_var;

  /// userobject that defines density as a fcn of porepressure
  const RichardsDensity & _density_UO;
};

#endif // RICHARDSDENSITYPRIMEAUX_H
