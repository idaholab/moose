//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"

/**
 * Computes a material property that is linear with respect to a list of aux variables
 *
 * This material is not meant to represent anything physical; it is used for
 * the testing of Jacobians of kernels, BCs, and materials where a material
 * property is supplied to those objects. The property is computed as
 * \f[
 *   y = b + \sum\limits_i m_i x_i ,
 * \f]
 * where \f$y\f$ is the new material property, \f$x_i\f$ are the aux variables,
 * \f$m_i\f$ are their slopes, and \f$b\f$ is the shift constant.
 */
class LinearTestMaterial : public DerivativeMaterialInterface<Material>
{
public:
  LinearTestMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Number of aux variables the material property depends upon
  const unsigned int _n_vars;

  /// List of aux variables the material property depends upon
  std::vector<const VariableValue *> _vars;

  /// Slopes with respect to the nonlinear variables
  const std::vector<Real> _slopes;

  /// Shift constant: 'b' in 'y = m * x + b'
  const Real _shift;

  /// Name of the new material property
  const MaterialPropertyName _y_name;

  /// Linear material property
  MaterialProperty<Real> & _y;

  /// Derivatives of material property with respect to each aux variable
  std::vector<MaterialProperty<Real> *> _y_derivatives;

public:
  static InputParameters validParams();
};
