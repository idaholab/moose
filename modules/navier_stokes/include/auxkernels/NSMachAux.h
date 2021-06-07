//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "AuxKernel.h"

// Forward Declarations
class SinglePhaseFluidProperties;

/**
 * Auxiliary kernel for computing the Mach number assuming an ideal gas.
 */
class NSMachAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NSMachAux(const InputParameters & parameters);

  virtual ~NSMachAux() {}

protected:
  virtual Real computeValue();

  /// Whether to use material properties instead of coupled variables to compute the Mach number
  const bool _use_mat_props;

  const VariableValue * const _u_vel;
  const VariableValue * const _v_vel;
  const VariableValue * const _w_vel;
  const VariableValue * const _specific_volume;
  const VariableValue * const _specific_internal_energy;

  /// speed
  const ADMaterialProperty<Real> * const _mat_speed;

  /// pressure
  const ADMaterialProperty<Real> * const _mat_pressure;

  /// fluid temperature
  const ADMaterialProperty<Real> * const _mat_T_fluid;

  // Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
