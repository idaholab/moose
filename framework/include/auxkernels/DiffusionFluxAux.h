//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DIFFUSIONFLUXAUX_H
#define DIFFUSIONFLUXAUX_H

#include "AuxKernel.h"

class DiffusionFluxAux;

template <>
InputParameters validParams<DiffusionFluxAux>();

/**
 * Auxiliary kernel responsible for computing the components of the flux vector
 * in diffusion problems
 */
class DiffusionFluxAux : public AuxKernel
{
public:
  DiffusionFluxAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// Will hold 0, 1, or 2 corresponding to x, y, or z.
  int _component;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// Holds the diffusivity from the material system
  const MaterialProperty<Real> & _diffusion_coef;
};

#endif // DIFFUSIONFLUXAUX_H
