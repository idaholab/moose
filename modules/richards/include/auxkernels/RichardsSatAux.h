/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSSATAUX_H
#define RICHARDSSATAUX_H

#include "AuxKernel.h"

#include "RichardsSat.h"

// Forward Declarations
class RichardsSatAux;

template <>
InputParameters validParams<RichardsSatAux>();

/**
 * Fluid Saturation as a function of effective saturation
 */
class RichardsSatAux : public AuxKernel
{
public:
  RichardsSatAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// effective saturation
  const VariableValue & _seff_var;

  /// User object defining saturation as a function of effective saturation
  const RichardsSat & _sat_UO;
};

#endif // RICHARDSSATAUX_H
