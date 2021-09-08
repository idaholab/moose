//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class INSFVVelocityVariable;

/*
 * Computes the value of the eddy viscosity for the mixing length model.
 */
class INSFVMixingLengthTurbulentViscosityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  INSFVMixingLengthTurbulentViscosityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// Turbulent eddy mixing length
  const VariableValue & _mixing_len;
};
