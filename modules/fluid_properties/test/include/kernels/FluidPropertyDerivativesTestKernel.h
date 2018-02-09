//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FLUIDPROPERTYDERIVATIVESTESTKERNEL_H
#define FLUIDPROPERTYDERIVATIVESTESTKERNEL_H

#include "Kernel.h"
#include "SinglePhaseFluidProperties.h"

class FluidPropertyDerivativesTestKernel;

template <>
InputParameters validParams<FluidPropertyDerivativesTestKernel>();

/**
 * Base class for kernels testing derivatives of a fluid property call
 *
 * To test the derivatives of a fluid property call, the call can be made in
 * computeQpResidual(), and the derivatives of that call can be put in
 * computeQpJacobian(). Then PetscJacobianTester can be used to compare to
 * the finite difference Jacobian.
 */
class FluidPropertyDerivativesTestKernel : public Kernel
{
public:
  FluidPropertyDerivativesTestKernel(const InputParameters & parameters);
  virtual ~FluidPropertyDerivativesTestKernel();

protected:
  virtual Real computeQpJacobian() override;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};

#endif /* FLUIDPROPERTYDERIVATIVESTESTKERNEL_H */
