//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declarations

/**
 * The DistributedForce kernel computes the body force due to the acceleration of mass in
 * an element. Following F=ma, the density must be integrated over the volume to yield the mass
 * of the volume. In this kernel, the acceleration is assumed to be a constant defined by up to
 * three dimensional components as input. The density must be obtained from the conservation of
 * mass equation as a coupled variable. Typical use of this kernel is the gravitational force
 * acting upon the fluid.
 */
class DistributedForce : public Kernel
{
public:
  static InputParameters validParams();

  DistributedForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _component;
  Real _acceleration;
  unsigned int _rho_var_number;
  const VariableValue & _rho;
};
