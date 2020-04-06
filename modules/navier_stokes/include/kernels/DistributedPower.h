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
 * The DistributedPower kernel computes the kinetic energy contribution of the body force due to
 * total energy in an element. In this kernel, the acceleration of the body force is assumed to be
 * a constant defined by up to three dimensional components as input. This kinetic energy
 * component is the acceleration vector dotted into the momentum vector components, yielding a
 * scalar kinetic contribution.
 */
class DistributedPower : public Kernel
{
public:
  static InputParameters validParams();

  DistributedPower(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  RealVectorValue _acceleration;
  unsigned int _rhou_var_number;
  unsigned int _rhov_var_number;
  unsigned int _rhow_var_number;
  const VariableValue & _rho_u;
  const VariableValue & _rho_v;
  const VariableValue & _rho_w;
};
