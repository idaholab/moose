//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  virtual Real computeValue() override;

  const VariableValue & _pressure;
  const VariableValue & _temperature;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* SPECIFICENTHALPYAUX_H */
