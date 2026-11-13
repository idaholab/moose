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
 * DSGShellMassEigenKernel provides the mass of a DSG shell element for one of its six
 * degrees of freedom, in the form the eigenvalue solvers expect: the kernel carries
 * the eigen vector tag and contributes the mass matrix, with the sign convention of
 * MassEigenKernel; it refuses to run without that tag.
 *
 * The translational mass is the consistent mass of the shell mid-surface, the
 * density times the thickness. The rotary inertia acts on the two rotations in the
 * plane of the element; the drilling rotation is given no inertia, as in
 * InertialForceDSGShell.
 */
class DSGShellMassEigenKernel : public Kernel
{
public:
  static InputParameters validParams();

  DSGShellMassEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Returns the entry of the in-plane projector between two global rotation axes
  Real inPlaneProjector(const unsigned int axis_a, const unsigned int axis_b) const;

  /// Returns the mass per unit area that multiplies the degree of freedom of this kernel
  Real massPerArea() const;

  /// The degree of freedom this kernel acts on: 0 to 2 for the displacements, 3 to 5 for the rotations
  const unsigned int _component;

  /// Whether the rotations in the plane of the element carry rotary inertia
  const bool _rotary_inertia;

  /// Variable numbers of the six degrees of freedom, in the order of the components
  std::vector<unsigned int> _variable_numbers;

  /// Values of the six degrees of freedom at the quadrature points
  std::vector<const VariableValue *> _values;

  /// Density of the shell material
  const MaterialProperty<Real> & _density;

  /// Thickness of the shell, from the section stiffness material
  const MaterialProperty<Real> & _thickness;

  /// Rotation matrix of the element frame, from the strain material
  const MaterialProperty<RankTwoTensor> & _rotation_matrix;
};
