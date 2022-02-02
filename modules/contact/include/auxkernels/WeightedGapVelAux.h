//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarNodalAuxKernel.h"

/**
 * Compute nodal weighted gap velocity based on a mortar discretization
 */
class WeightedGapVelAux : public MortarNodalAuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  WeightedGapVelAux(const InputParameters & parameters);

protected:
  Real computeValue() override;

  void computeQpProperties();

  void computeQpIProperties();

  /// For 2D mortar contact no displacement will be specified, so const pointers used
  const bool _has_disp_z;

  /// Reference to the secondary variable
  const MooseVariable & _disp_x;

  const MooseVariable & _disp_y;

  const MooseVariable * _disp_z;

  /// x-velocity on the secondary face
  const ADVariableValue & _secondary_x_dot;

  /// x-velocity on the primary face
  const ADVariableValue & _primary_x_dot;

  /// y-velocity on the secondary face
  const ADVariableValue & _secondary_y_dot;

  /// y-velocity on the primary face
  const ADVariableValue & _primary_y_dot;

  /// z-velocity on the secondary face
  const ADVariableValue * const _secondary_z_dot;

  /// z-velocity on the primary face
  const ADVariableValue * const _primary_z_dot;

  /// The weighted gap velocity
  Real _weighted_gap_velocity;

  /// The gap velocity vector at the current quadrature point, used when we are interpolating the normal
  /// vector, multiplied by the normals vector evaluated at the current quadrature point and JxW
  Real _qp_gap_velocity;

  /// The gap velocity vector at the current quadrature point, used when we are not interpolating the normal
  /// vector, multipled by JxW
  RealVectorValue _qp_gap_velocity_nodal;

  /// The current test function index
  unsigned int _i;

  /// The current quadrature point index
  unsigned int _qp;
};
