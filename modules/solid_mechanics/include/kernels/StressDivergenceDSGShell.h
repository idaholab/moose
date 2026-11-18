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
#include "DenseMatrix.h"
#include "libmesh/dense_vector.h"

/**
 * StressDivergenceDSGShell computes the internal force of a DSG shell element
 * for one of its six degrees of freedom, and the exact Jacobian of that force.
 *
 * The residual is the product of the strain-displacement matrix with the stress
 * resultants, and the Jacobian the product of the strain-displacement matrix
 * with the section stiffness and the strain-displacement matrix again. Both are
 * written out because the element is linear in its degrees of freedom.
 */
class StressDivergenceDSGShell : public Kernel
{
public:
  static InputParameters validParams();

  StressDivergenceDSGShell(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  virtual void precalculateJacobian() override;
  virtual void precalculateOffDiagJacobian(unsigned int jvar) override;

  /**
   * Returns the entry of the element stiffness at this quadrature point between the test
   * function of this kernel's component and the trial function of another component.
   */
  Real stiffnessEntry(const unsigned int other_component) const;

  /// The section stiffness times the strain-displacement matrix, per quadrature point of the
  /// current element, formed before the Jacobian loops
  std::vector<DenseMatrix<Real>> _stiffness_times_matrix;

  /// The degree of freedom this kernel acts on: 0 to 2 for the displacements, 3 to 5 for the rotations
  const unsigned int _component;

  /// Variable numbers of the six degrees of freedom, in the order of the components
  std::vector<unsigned int> _variable_numbers;

  /// Strain-displacement matrix from the strain material
  const MaterialProperty<DenseMatrix<Real>> & _strain_displacement_matrix;

  /// Stress resultants from the resultants material
  const MaterialProperty<DenseVector<Real>> & _resultants;

  /// Section stiffness from the section stiffness material
  const MaterialProperty<DenseMatrix<Real>> & _section_stiffness;
};
