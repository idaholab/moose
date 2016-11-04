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

#ifndef ANISOTROPICDIFFUSION_H
#define ANISOTROPICDIFFUSION_H

#include "Kernel.h"

class AnisotropicDiffusion;

template <>
InputParameters validParams<AnisotropicDiffusion>();

/**
 * This kernel implements the Laplacian operator
 * multiplied by a 2nd order tensor giving
 * anisotropic (direction specific) diffusion:
 * $\overline K \cdot \nabla u \cdot \nabla \phi_i$
 */
class AnisotropicDiffusion : public Kernel
{
public:
  AnisotropicDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  RealTensorValue _k;
};

#endif /* ANISOTROPICDIFFUSION_H */
