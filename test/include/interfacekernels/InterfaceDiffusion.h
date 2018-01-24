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

#ifndef INTERFACEDIFFUSION_H
#define INTERFACEDIFFUSION_H

#include "InterfaceKernel.h"

// Forward Declarations
class InterfaceDiffusion;

template <>
InputParameters validParams<InterfaceDiffusion>();

/**
 * DG kernel for interfacing diffusion between two variables on adjacent blocks
 */
class InterfaceDiffusion : public InterfaceKernel
{
public:
  InterfaceDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  Real _D;
  Real _D_neighbor;
};

#endif
