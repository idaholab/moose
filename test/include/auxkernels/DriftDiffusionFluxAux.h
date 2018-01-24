//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DRIFTDIFFUSIONFLUXAUX_H
#define DRIFTDIFFUSIONFLUXAUX_H

#include "AuxKernel.h"

class DriftDiffusionFluxAux;

template <>
InputParameters validParams<DriftDiffusionFluxAux>();

class DriftDiffusionFluxAux : public AuxKernel
{
public:
  DriftDiffusionFluxAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  const Real _sgn;
  const VariableGradient & _grad_potential;
  const VariableValue & _u;
  const VariableGradient & _grad_u;
  const int _component;
};

#endif // DRIFTDIFFUSIONFLUXAUX_H
