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

#ifndef COEFFPARAMDIFFUSION_H
#define COEFFPARAMDIFFUSION_H

// Including the "Diffusion" Kernel here so we can extend it
#include "Diffusion.h"

class CoeffParamDiffusion;

template <>
InputParameters validParams<CoeffParamDiffusion>();

class CoeffParamDiffusion : public Diffusion
{
public:
  CoeffParamDiffusion(const InputParameters & parameters);
  virtual ~CoeffParamDiffusion();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _D;
};

#endif /* COEFFPARAMDIFFUSION_H */
