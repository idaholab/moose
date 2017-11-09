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
