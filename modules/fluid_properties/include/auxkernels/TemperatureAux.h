//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TEMPERATUREAUX_H
#define TEMPERATUREAUX_H

#include "AuxKernel.h"

class TemperatureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<TemperatureAux>();

/**
 * Compute temperature values from specific volume and internal energy
 */
class TemperatureAux : public AuxKernel
{
public:
  TemperatureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _v;
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* TEMPERATUREAUX_H */
