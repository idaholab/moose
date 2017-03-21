/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SPECIFICENTHALPYAUX_H
#define SPECIFICENTHALPYAUX_H

#include "AuxKernel.h"

class SpecificEnthalpyAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<SpecificEnthalpyAux>();

/**
 * Computes specific enthalpy from pressure and temperature.
 */
class SpecificEnthalpyAux : public AuxKernel
{
public:
  SpecificEnthalpyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _pressure;
  const VariableValue & _temperature;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* SPECIFICENTHALPYAUX_H */
