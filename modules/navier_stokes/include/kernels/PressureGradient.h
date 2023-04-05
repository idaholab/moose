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

/**
 * Adds the pressure gradient term to the Navier-Stokes momentum equation
 */
class PressureGradient : public Kernel
{
public:
  static InputParameters validParams();

  PressureGradient(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Whether to integrate the pressure term by parts
  const bool _integrate_p_by_parts;

  /// The velocity component this object is adding a residual for
  unsigned int _component;

  /// The pressure value
  const VariableValue & _pressure;

  /// The pressure gradient
  const VariableGradient & _grad_pressure;

  /// The number of the pressure variable
  const unsigned int _pressure_id;

  /// The coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// The radial coordinate index for RZ coordinate systems
  const unsigned int _rz_radial_coord;
};
