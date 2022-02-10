//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGKernel.h"

class NumericalFlux3EqnBase;

/**
 * Adds side fluxes for the 1-D, 1-phase, variable-area Euler equations
 *
 * Note that this kernel is not responsible for the calculation of the side
 * fluxes - this is handled by a user object passed to this kernel.
 *
 * The side fluxes are computed using the reconstructed linear solution, passed
 * in using a material, but their Jacobians are approximated using the constant
 * monomial cell-average solution, coupled in as variables. This approximation
 * has been found to be perfectly sufficient and avoids the complexities and
 * expense of the perfect Jacobian, which would require chain rule with the
 * Jacobians of the solution slopes.
 */
class NumericalFlux3EqnDGKernel : public DGKernel
{
public:
  NumericalFlux3EqnDGKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /**
   * Creates the mapping of coupled variable index to index in Euler system
   *
   * @returns the mapping of coupled variable index to index in Euler system
   */
  std::map<unsigned int, unsigned int> getIndexMapping() const;

  /// Area
  const VariableValue & _A_elem;
  const VariableValue & _A_neig;

  // reconstructed variable values in this cell
  const MaterialProperty<Real> & _rhoA1;
  const MaterialProperty<Real> & _rhouA1;
  const MaterialProperty<Real> & _rhoEA1;
  const MaterialProperty<Real> & _p1;

  // reconstructed variable values in neighbor cell
  const MaterialProperty<Real> & _rhoA2;
  const MaterialProperty<Real> & _rhouA2;
  const MaterialProperty<Real> & _rhoEA2;
  const MaterialProperty<Real> & _p2;

  /// Numerical flux user object
  const NumericalFlux3EqnBase & _numerical_flux;

  // coupled variable indices
  const unsigned int _rhoA_var;
  const unsigned int _rhouA_var;
  const unsigned int _rhoEA_var;

  /// map of coupled variable index to equations variable index convention
  const std::map<unsigned int, unsigned int> _jmap;

  /// index within the Euler system of the equation upon which this kernel acts
  const unsigned int _equation_index;

public:
  static InputParameters validParams();
};
