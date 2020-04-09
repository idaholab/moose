//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSKernel.h"
#include "NSTemperatureDerivs.h"

// ForwardDeclarations

/**
 * This class is responsible for computing residuals and Jacobian
 * terms for the k * grad(T) * grad(phi) term in the Navier-Stokes
 * energy equation.
 */
class NSEnergyThermalFlux : public NSKernel
{
public:
  static InputParameters validParams();

  NSEnergyThermalFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Gradients
  const VariableGradient & _grad_temp;

  // Material properties
  const MaterialProperty<Real> & _thermal_conductivity;

  // A helper object for computing temperature gradient and Hessians.
  // Constructed via a reference to ourself so we can access all of our data.
  NSTemperatureDerivs<NSEnergyThermalFlux> _temp_derivs;

  // Declare ourselves a friend to the helper class
  template <class U>
  friend class NSTemperatureDerivs;

private:
  // Computes the Jacobian value (on or off-diagonal) for
  // var_number, which has been mapped to
  // 0 = rho
  // 1 = rho*u
  // 2 = rho*v
  // 3 = rho*w
  // 4 = rho*E
  Real computeJacobianHelper_value(unsigned var_number);

  // Single vector to refer to all gradients.  We have to store
  // pointers since you can't have a vector<Foo&>.  Initialized in
  // the ctor.
  std::vector<const VariableGradient *> _gradU;
};
