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
 * Compute worn-out depth based on Archard's wear law
 */
class MortarArchardsLawAux : public MortarNodalAuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor of mortar auxiliary kernel to compute nodal worn-out depth.
   */
  MortarArchardsLawAux(const InputParameters & parameters);

protected:
  Real computeValue() override;

  void computeQpProperties();

  void computeQpIProperties();

  // Mortar normal contact pressure
  const VariableValue & _normal_pressure;

  /// Friction coefficient used to compute Archard's law
  const Real _friction_coefficient;

  /// Typically a constant used to characterize wear behavior of a material surface
  const Real _energy_wear_coefficient;

  /// Displacement variables
  const std::vector<const MooseVariable *> _displacements;

  /// For 2D mortar contact no displacement will be specified, so const pointers used
  const bool _has_disp_z;

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

  /// The worn-out depth due to Archard's law (Aux Kernel computed value)
  Real _worn_depth;

  /// The gap velocity vector at the current quadrature point, used when we are not interpolating the normal
  /// vector, multipled by JxW
  RealVectorValue _qp_gap_velocity_nodal;
};
