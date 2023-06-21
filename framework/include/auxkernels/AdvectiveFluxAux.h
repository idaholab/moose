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
 * Auxiliary kernel responsible for computing the components of the advection flux vector
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

  /// Holds the solution at the current quadrature points
  const VariableValue & _advected_variable;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;

  /// Velocity components
  const Moose::Functor<ADReal> & _vel_x;
  const Moose::Functor<ADReal> * _vel_y;
  const Moose::Functor<ADReal> * _vel_z;

  /// Whether an advected variable was supplied in the input
  const bool _advected_variable_supplied;

  /// Whether an advected material property was supplied in the input
  const bool _advected_mat_prop_supplied;

  /// Material property storing the advected quantity; used for finite elements
  const Moose::Functor<ADReal> & _advected_material_property;
};
