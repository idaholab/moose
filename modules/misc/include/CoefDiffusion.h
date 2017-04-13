/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COEFDIFFUSION_H
#define COEFDIFFUSION_H

#include "Kernel.h"
#include "Function.h"

// Forward Declarations
class CoefDiffusion;

template <>
InputParameters validParams<CoefDiffusion>();

class CoefDiffusion : public Kernel
{
public:
  CoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  const Real _coef;
  Function * const _func;
};

#endif // COEFDIFFUSION_H
