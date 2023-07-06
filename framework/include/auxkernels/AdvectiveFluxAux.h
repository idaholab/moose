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

/**
 * Auxiliary kernel responsible for computing a component of the advection flux vector
 */
class AdvectiveFluxAux : public AuxKernel
{
public:
  static InputParameters validParams();

  AdvectiveFluxAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Whether the normal component has been selected
  const bool _use_normal;

  /// Will hold 0, 1, or 2 corresponding to x, y, or z.
  const int _component;

  /// Functor for the scalar field advected quantity (for a variable usually)
  const Moose::Functor<Real> & _advected_quantity;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;

  /// Velocity components
  const Moose::Functor<Real> & _vel_x;
  const Moose::Functor<Real> * const _vel_y;
  const Moose::Functor<Real> * const _vel_z;

  /// Whether an advected quantity was supplied in the input
  const bool _advected_quantity_supplied;

  /// Whether an advected material property was supplied in the input
  const bool _advected_mat_prop_supplied;

  /// Material property storing the advected quantity; used for finite elements
  const MaterialProperty<Real> & _advected_material_property;
};
