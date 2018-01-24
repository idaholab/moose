/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSRELPERMPRIMEPRIMEAUX_H
#define RICHARDSRELPERMPRIMEPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsRelPerm.h"

// Forward Declarations
class RichardsRelPermPrimePrimeAux;

template <>
InputParameters validParams<RichardsRelPermPrimePrimeAux>();

/**
 * Relative Permeability as a function of effective saturation
 */
class RichardsRelPermPrimePrimeAux : public AuxKernel
{
public:
  RichardsRelPermPrimePrimeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  const VariableValue & _seff_var;

  /// userobject that defines relative permeability function
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSRELPERMPRIMEPRIMEAUX_H
