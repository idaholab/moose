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

#ifndef FLUXAVERAGEAUX_H
#define FLUXAVERAGEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class FluxAverageAux;

template <>
InputParameters validParams<FluxAverageAux>();

/**
 * Computes the average "flux" of a coupled variable and puts it in an elemental field.
 *
 * Only works on boundaries.
 */
class FluxAverageAux : public AuxKernel
{
public:
  FluxAverageAux(const InputParameters & parameters);

  virtual ~FluxAverageAux() {}

protected:
  virtual Real computeValue();

  /// The "material" property
  Real _diffusivity;

  /// Coupled gradient
  const VariableGradient & _coupled_gradient;

  /// The variable we're coupled to
  MooseVariable & _coupled_var;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;
};

#endif // FLUXAVERAGEAUX_H
