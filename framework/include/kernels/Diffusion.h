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

#ifndef DIFFUSION_H
#define DIFFUSION_H

#include "Kernel.h"

class Diffusion;

template <>
InputParameters validParams<Diffusion>();

/**
 * This kernel implements the Laplacian operator:
 * $\nabla u \cdot \nabla \phi_i$
 */
class Diffusion : public Kernel
{
public:
  Diffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;
};

#endif /* DIFFUSION_H */
