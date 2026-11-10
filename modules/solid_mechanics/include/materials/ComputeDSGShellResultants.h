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
#include "libmesh/dense_vector.h"

/**
 * ComputeDSGShellResultants computes the stress resultants of a DSG shell
 * element from its generalized strains and its section stiffness: the membrane
 * forces, the bending moments, the transverse shear forces and the drilling
 * moment, all per unit length and in the element frame.
 */
class ComputeDSGShellResultants : public Material
{
public:
  static InputParameters validParams();

  ComputeDSGShellResultants(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Generalized strains from the strain material
  const MaterialProperty<DenseVector<Real>> & _generalized_strain;

  /// Section stiffness from the section stiffness material
  const MaterialProperty<DenseMatrix<Real>> & _section_stiffness;

  /// Stress resultants: N11 N22 N12 M11 M22 M12 Q1 Q2 and the drilling moment
  MaterialProperty<DenseVector<Real>> & _resultants;

  /// Generalized strains at the nodes, present when the strain material was asked for them
  const OptionalMaterialProperty<DenseMatrix<Real>> & _nodal_generalized_strain;

  /// Resultants at the nodes, one row per node, at the first quadrature point of the element;
  /// empty when the nodal strains are not computed
  MaterialProperty<DenseMatrix<Real>> & _nodal_resultants;
};
