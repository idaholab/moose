/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSRELPERMAUX_H
#define RICHARDSRELPERMAUX_H

#include "AuxKernel.h"

#include "RichardsRelPerm.h"

// Forward Declarations
class RichardsRelPermAux;

template <>
InputParameters validParams<RichardsRelPermAux>();

/**
 * Relative Permeability as a function of effective saturation
 */
class RichardsRelPermAux : public AuxKernel
{
public:
  RichardsRelPermAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  const VariableValue & _seff_var;

  /// userobject that defines relative permeability function
  const RichardsRelPerm & _relperm_UO;
};

#endif // RICHARDSRELPERMAUX_H
