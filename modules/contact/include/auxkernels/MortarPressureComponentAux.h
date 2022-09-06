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
 * Transforms a Cartesian Lagrange multiplier vector, typically employed for mortar mechanical
 * contact,  to local coordinates and outputs each individual component along the normal or
 * tangential direction.
 */
class MortarPressureComponentAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MortarPressureComponentAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Lagrange multiplier variable along the x direction
  const MooseArray<Real> * const _lm_var_x;

  /// Lagrange multiplier variable along the y direction
  const MooseArray<Real> * const _lm_var_y;

  /// Lagrange multiplier variable along the z direction (3D)
  const MooseArray<Real> * const _lm_var_z;

  /// Fe problem to obtain primary/secondary ids
  const FEProblemBase & _fe_problem;

  /// Boundary ID for the primary surface
  const BoundaryID _primary_id;

  /// Boundary ID for the secondary surface
  const BoundaryID _secondary_id;

  /// The component of the Lagrange multiplier to compute
  const enum class ComponentType { NORMAL, TANGENT1, TANGENT2 } _component;

  /// Whether to use displaced mesh (required for this auxiliary kernel)
  const bool _use_displaced_mesh;

  /// Handle to mortar generation object to obtain mortar geometry
  const AutomaticMortarGeneration * _mortar_generation_object;
};
