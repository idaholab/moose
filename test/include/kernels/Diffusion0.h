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
#ifndef DIFFUSION0_H
#define DIFFUSION0_H

#include "Kernel.h"
#include "Material.h"

// Forward Declarations
class Diffusion0;

template <>
InputParameters validParams<Diffusion0>();

class Diffusion0 : public Kernel
{
public:
  Diffusion0(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /// Parameters for spatially linearly varying diffusivity.
  Real _Ak, _Bk, _Ck;
};

#endif // DIFFUSION0_H
