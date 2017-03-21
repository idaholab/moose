/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
