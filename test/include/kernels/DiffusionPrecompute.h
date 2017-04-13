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
#ifndef DIFFUSIONPRECOMPUTE_H
#define DIFFUSIONPRECOMPUTE_H

#include "KernelGrad.h"

class DiffusionPrecompute;

template <>
InputParameters validParams<DiffusionPrecompute>();

class DiffusionPrecompute : public KernelGrad
{
public:
  DiffusionPrecompute(const InputParameters & parameters);
  virtual ~DiffusionPrecompute();

protected:
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
};

#endif /* DIFFUSIONPRECOMPUTE_H */
