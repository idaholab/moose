//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PRESSUREAUX_H
#define PRESSUREAUX_H

#include "AuxKernel.h"

class PressureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<PressureAux>();

class PressureAux : public AuxKernel
{
public:
  PressureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _s_volume;
  const VariableValue & _s_internal_energy;
  const SinglePhaseFluidProperties & _fp;
};

#endif /* PRESSUREAUX_H */
