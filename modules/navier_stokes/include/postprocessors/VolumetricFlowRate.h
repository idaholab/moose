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
#include "SideIntegralPostprocessor.h"

// Forward Declarations

/**
 * This postprocessor computes the volumetric flow rate through a boundary.
 */
class VolumetricFlowRate : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  VolumetricFlowRate(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Whether FV variables are used, for all variables
  bool _fv;

  /// Velocity components
  const VariableValue & _vel_x;
  const VariableValue & _vel_y;
  const VariableValue & _vel_z;

  /// Velocity components for finite volume
  const MooseVariableFV<Real> * const _fv_vel_x;
  const MooseVariableFV<Real> * const _fv_vel_y;
  const MooseVariableFV<Real> * const _fv_vel_z;

  /// Advected quantities
  const VariableValue & _advected_variable;
  const MooseVariableFV<Real> * const _fv_advected_variable;
  const ADMaterialProperty<Real> & _advected_material_property;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;
};
