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

// Forward declare variable class
class INSFVVelocityVariable;

class INSFVMixingLengthReynoldsStress : public FVFluxKernel
{
public:
  static InputParameters validParams();

  INSFVMixingLengthReynoldsStress(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// index x|y|z
  const unsigned int _axis_index;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// Density
  const Real & _rho;

  /// Turbulent eddy mixing length
  const VariableValue & _mixing_len;
  /// Turbulent eddy mixing length for the neighbor cell
  const VariableValue & _mixing_len_neighbor;
};
