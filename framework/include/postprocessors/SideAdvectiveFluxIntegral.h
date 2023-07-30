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
#include "MooseVariableInterface.h"

// Forward Declarations
template <bool>
class SideAdvectiveFluxIntegralTempl;
typedef SideAdvectiveFluxIntegralTempl<false> SideAdvectiveFluxIntegral;
typedef SideAdvectiveFluxIntegralTempl<true> ADSideAdvectiveFluxIntegral;

/**
 * This postprocessor computes a side integral of the mass flux.
 */
template <bool is_ad>
class SideAdvectiveFluxIntegralTempl : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  SideAdvectiveFluxIntegralTempl(const InputParameters & parameters);

protected:
  Real computeQpIntegral() override;
  Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  /// Whether the normal component has been selected
  const bool _use_normal;

  /// Will hold 0, 1, or 2 corresponding to x, y, or z.
  const int _component;

  /// Whether an advected variable was supplied in the input
  const bool _advected_variable_supplied;

  /// Variable storing the advected quantity; used for finite elements
  const VariableValue & _advected_variable;

  /// Whether an advected material property was supplied in the input
  const bool _advected_mat_prop_supplied;

  /// Material property storing the advected quantity; used for finite elements
  const GenericMaterialProperty<Real, is_ad> & _advected_material_property;

  /// The functor representing the advected quantity for finite volume
  const Moose::Functor<Real> * const _adv_quant;

  /// Velocity components
  const Moose::Functor<Real> & _vel_x;
  const Moose::Functor<Real> * const _vel_y;
  const Moose::Functor<Real> * const _vel_z;
};
