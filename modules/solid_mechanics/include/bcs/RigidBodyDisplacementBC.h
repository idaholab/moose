//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirichletBCBase.h"

class Function;

/**
 * Prescribes one component of the displacement of a rigid body undergoing a finite translation
 * and rotation, U = t + (R(theta) - I) (X - X_ref), where the translation t and the rotation
 * vector theta are given by functions of time.
 */
class RigidBodyDisplacementBC : public DirichletBCBase
{
public:
  static InputParameters validParams();

  RigidBodyDisplacementBC(const InputParameters & parameters);

protected:
  virtual Real computeQpValue() override;

  /// Displacement component prescribed by this boundary condition
  const unsigned int _component;

  /// Point about which the body rotates, in the reference configuration
  const Point _reference_point;

  /// Functions giving the components of the translation of the reference point
  std::vector<const Function *> _translations;

  /// Functions giving the components of the rotation vector (only the z component in 2D)
  std::vector<const Function *> _rotations;
};
