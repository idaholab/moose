//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "Material.h"

// Forward declaration
class ComputeFalseMaterial;

template <>
InputParameters validParams<ComputeFalseMaterial>();

/**
 * ComputeFalseMaterial is an example of a compute = false Material that computes a vector
 * of Properties and a Real-number Property, both of which have Old values
 */
class ComputeFalseMaterial : public Material
{
public:
  ComputeFalseMaterial(const InputParameters & parameters);

  /// sets our _qp to qp.  Our _qp may be different from the Material that is using the Properties from this Material
  void setQp(unsigned qp);

  /// Compute the vector and real-number of Properties at _qp
  void computeQpThings();

  ///@{ Retained as empty methods to avoid a warning from Material.C in framework. These methods are unused in all inheriting classes and should not be overwritten.
  void resetQpProperties() final {}
  void resetProperties() final {}
  ///@}

protected:
  virtual void initQpStatefulProperties() override;

  /// size of the vector
  const unsigned _vector_size;

  /// the vector computed by this Material
  MaterialProperty<std::vector<Real>> & _compute_false_vector;

  /// the old values of the vector computed by this Material
  const MaterialProperty<std::vector<Real>> & _compute_false_vector_old;

  /// the scalar computed by this Material
  MaterialProperty<Real> & _compute_false_scalar;

  /// the old values of the scalar computed by this Material
  const MaterialProperty<Real> & _compute_false_scalar_old;
};
