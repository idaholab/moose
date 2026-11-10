//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"

#include "MooseVariable.h"

/**
 * Element frame and relative deformation of a two-node spring element.
 *
 * The spring is an EDGE2 element between nodes 0 and 1, of any nonzero length (a spring between
 * coincident positions is a short element with local_axis_1). The
 * material builds the local frame (axis 1 along the element or along local_axis_1, axis 2 the
 * projection of local_axis_2 on the plane normal to axis 1) and reports the translation and
 * the rotation of node 1 relative to node 0 in that frame, tension positive. The spring law
 * materials turn the deformation into a force; StressDivergenceSpring applies it.
 */
class ComputeSpringDeformation : public Material
{
public:
  static InputParameters validParams();

  ComputeSpringDeformation(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
  virtual void computeQpProperties() override {}

  /// Rotation matrix of the current element, rows are the local axes in global components
  RankTwoTensor computeRotationMatrix() const;

  /// Reports an element of zero length on the blocks of this material
  void checkElementLengths() const;

  /// Number of displacement and rotation variables
  const unsigned int _ndisp;
  const unsigned int _nrot;

  /// Whether the deformation rates are computed from the time derivative of the solution
  const bool _compute_rates;

  /// Prefix of the property names, for several springs on one block
  const std::string _base_name;

  /// Nodal values of the displacement and rotation variables on the current element
  std::vector<const VariableValue *> _disp_dof_values;
  std::vector<const VariableValue *> _rot_dof_values;

  /// Displacement and rotation variables, for the time derivatives of their nodal values
  std::vector<MooseVariable *> _disp_vars;
  std::vector<MooseVariable *> _rot_vars;

  /// Whether the local axes were given, and their values
  const bool _has_local_axis_1;
  const bool _has_local_axis_2;
  const RealVectorValue _local_axis_1;
  const RealVectorValue _local_axis_2;

  /// Rotation matrix, deformations and their rates
  MaterialProperty<RankTwoTensor> & _rotation_matrix;
  MaterialProperty<RealVectorValue> & _deformation;
  MaterialProperty<RealVectorValue> * const _rotation;
  MaterialProperty<RealVectorValue> * const _deformation_rate;
  MaterialProperty<RealVectorValue> * const _rotation_rate;
  MaterialProperty<Real> * const _rate_du_dot_du;
};
