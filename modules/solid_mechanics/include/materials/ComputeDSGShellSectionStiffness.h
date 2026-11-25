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
#include "DenseMatrix.h"

/**
 * ComputeDSGShellSectionStiffness computes the linear elastic section stiffness
 * of a DSG shell element: the matrix that relates the generalized strains to
 * the stress resultants of a homogeneous isotropic plate of constant thickness.
 *
 * The matrix is block diagonal: a membrane block, a bending block, a transverse
 * shear block and the drilling stiffness. On the three-node element the shear
 * block carries the thin-limit factor of Lyly, Stenberg and Vihinen (1993),
 * which keeps that element accurate for thin plates.
 */
class ComputeDSGShellSectionStiffness : public Material
{
public:
  static InputParameters validParams();

  ComputeDSGShellSectionStiffness(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Young's modulus of the shell material
  const Real _youngs_modulus;

  /// Poisson's ratio of the shell material
  const Real _poissons_ratio;

  /// Thickness of the shell
  const Real _thickness;

  /// Shear correction factor of the transverse shear stiffness
  const Real _shear_correction_factor;

  /// Factor on the drilling stiffness G t

  const Real _drilling_stiffness_factor;

  /// Constant of the thin-limit factor applied to the shear stiffness of the three-node element
  const Real _thin_limit_factor;

  /// Longest edge of the element, from the strain material
  const MaterialProperty<Real> & _longest_edge;

  /// Section stiffness (9 by 9) relating the generalized strains to the stress resultants
  MaterialProperty<DenseMatrix<Real>> & _section_stiffness;

  /// Thickness of the shell, stored for the objects that need it
  MaterialProperty<Real> & _thickness_property;
};
