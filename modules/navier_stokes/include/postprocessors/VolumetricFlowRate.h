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
#include "MathFVUtils.h"

class RhieChowFaceFluxProvider;

/**
 * This postprocessor computes the volumetric flow rate through a boundary, internal or external to
 * the flow domain.
 */
class VolumetricFlowRate : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  VolumetricFlowRate(const InputParameters & parameters);

  /// Currently only requests some boundary data from the RhieChow interpolator
  void initialSetup() override;
  void meshChanged() override;

protected:
  Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  Real computeQpIntegral() override;

  /// Velocity components
  const VariableValue & _vel_x;
  const VariableValue & _vel_y;
  const VariableValue & _vel_z;

  /// Whether an advected variable was supplied in the input
  const bool _advected_variable_supplied;
  /// Variable storing the advected quantity; used for finite elements
  const VariableValue & _advected_variable;

  /// Whether an advected material property was supplied in the input
  const bool _advected_mat_prop_supplied;
  /// Material property storing the advected quantity; used for finite elements
  const Moose::Functor<ADReal> & _advected_material_property;

  /// The functor representing the advected quantity for finite volume
  const Moose::Functor<ADReal> * const _adv_quant;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  /// The interpolation method to use for the velocity
  Moose::FV::InterpMethod _velocity_interp_method;

  /// The Rhie-Chow interpolation user object
  const RhieChowFaceFluxProvider * const _rc_uo;

  /// If the velocity of the potentially moving mesh should be subtracted or not
  const bool _subtract_mesh_velocity;
};
