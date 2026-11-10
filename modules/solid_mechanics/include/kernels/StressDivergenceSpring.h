//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "RankTwoTensor.h"

/**
 * Internal force of a two-node spring element on one of its six degrees of freedom.
 *
 * The force and the tangent stiffness come from a spring law material in the local frame of
 * ComputeSpringDeformation; the kernel rotates them to the global axes and writes the
 * residual of the two nodes directly, as the truss kernel does: the spring pulls node 1
 * toward node 0 with the force and pushes node 0 the other way.
 */
class StressDivergenceSpring : public Kernel
{
public:
  static InputParameters validParams();

  StressDivergenceSpring(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override { return 0.0; }
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /// Tangent stiffness of the element in global axes, (axis, axis) entries of the rotated law
  RankTwoTensor globalStiffness() const;

  /// Fills the local 2 x 2 element matrix of this variable against a coupled component
  void fillLocalMatrix(const RankTwoTensor & global_stiffness,
                       unsigned int coupled_axis,
                       unsigned int phi_size);

  /// Degree of freedom this kernel acts on, 0 to 5
  const unsigned int _component;

  /// Global axis of that degree of freedom, 0 to 2
  const unsigned int _axis;

  /// Whether the kernel acts on a rotation
  const bool _rotational;

  /// Prefix of the property names, the base_name of the spring
  const std::string _base_name;

  /// Variable numbers of the three variables of the same kind (displacements or rotations)
  std::vector<unsigned int> _group_vars;

  /// Rotation matrix, force (moment) and stiffness of the law in the local frame
  const MaterialProperty<RankTwoTensor> & _rotation_matrix;
  const MaterialProperty<RealVectorValue> & _force;
  const MaterialProperty<RankTwoTensor> & _stiffness;
};
