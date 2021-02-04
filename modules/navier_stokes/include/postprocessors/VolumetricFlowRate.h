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
#include "FVUtils.h"

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
  const bool _fv;

  /// Velocity components
  const VariableValue & _vel_x;
  const VariableValue & _vel_y;
  const VariableValue & _vel_z;

  /// Velocity components for finite volume
  const MooseVariableFV<Real> * const _fv_vel_x;
  const MooseVariableFV<Real> * const _fv_vel_y;
  const MooseVariableFV<Real> * const _fv_vel_z;

  /// Whether an advected variable was supplied in the input
  const bool _advected_variable_supplied;
  /// Variable storing the advected quantity
  const VariableValue & _advected_variable;
  /// Variable storing the advected quantity for finite volume
  const MooseVariableFV<Real> * const _fv_advected_variable;

  /// Whether an advected material property was supplied in the input
  const bool _advected_mat_prop_supplied;
  /// Material property storing the advected quantity
  const ADMaterialProperty<Real> & _advected_material_property;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;
};
