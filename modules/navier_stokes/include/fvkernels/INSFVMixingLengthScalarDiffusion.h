//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"
#include "INSFVVelocityVariable.h"

class INSFVMixingLengthScalarDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();

  INSFVMixingLengthScalarDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

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
  /// Turbulent eddy mixing length for the neighbor cell
  const VariableValue & _mixing_len_neighbor;

  /// Turbulent Schmidt number (or turbulent Prandtl number)
  const Real & _schmidt_number;
};
