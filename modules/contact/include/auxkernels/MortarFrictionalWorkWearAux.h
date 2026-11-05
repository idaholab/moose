//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarNodalAuxKernel.h"

/**
 * Compute the nodal worn-out depth from Reye's wear law (Reye 1860, col. 239): the normal wear of
 * a surface element is proportional to its frictional work, i.e. to normal pressure times sliding
 * velocity. The pressure-velocity form is commonly called Archard's law.
 */
class MortarFrictionalWorkWearAux : public MortarNodalAuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor of mortar auxiliary kernel to compute nodal worn-out depth.
   */
  MortarFrictionalWorkWearAux(const InputParameters & parameters);

protected:
  Real computeValue() override;

  void computeQpProperties();

  void computeQpIProperties();

  // Mortar normal contact pressure
  const VariableValue & _normal_pressure;

  /// Coulomb friction coefficient used to compute the frictional dissipation rate
  const Real _friction_coefficient;

  /// Worn-out depth per unit dissipated frictional energy density (Reye's wear coefficient)
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

  /// The worn-out depth (Aux Kernel computed value)
  Real _worn_depth;

  /// The gap velocity vector at the current quadrature point, used when we are not interpolating the normal
  /// vector, multipled by JxW
  RealVectorValue _qp_gap_velocity_nodal;
};
