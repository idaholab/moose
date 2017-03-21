/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSRELPERMPRIMEAUX_H
#define RICHARDSRELPERMPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsRelPerm.h"

// Forward Declarations
class RichardsRelPermPrimeAux;

template <>
InputParameters validParams<RichardsRelPermPrimeAux>();

/**
 * Derivative of relative Permeability wrt effective saturation
 */
class RichardsRelPermPrimeAux : public AuxKernel
{
public:
  RichardsRelPermPrimeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  const VariableValue & _seff_var;

  /// userobject that defines relative permeability function
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSRELPERMPRIMEAUX_H
