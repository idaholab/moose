//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SATURATIONTEMPERATUREAUX_H
#define SATURATIONTEMPERATUREAUX_H

#include "AuxKernel.h"

class SaturationTemperatureAux;
class TwoPhaseFluidProperties;

template <>
InputParameters validParams<SaturationTemperatureAux>();

/**
 * Computes saturation temperature from pressure and 2-phase fluid properties object
 */
class SaturationTemperatureAux : public AuxKernel
{
public:
  SaturationTemperatureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Pressure at which to evaluate saturation temperature
  const VariableValue & _p;
  /// 2-phase fluid properties object
  const TwoPhaseFluidProperties & _fp_2phase;
};

#endif /* SATURATIONTEMPERATUREAUX_H */
