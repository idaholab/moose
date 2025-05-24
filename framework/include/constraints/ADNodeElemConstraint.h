//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeElemConstraintBase.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"

/**
 * An ADNodeElemConstraint is used when you need to create constraints between a subdomain of
 * secondary nodes and a subdomain of primary elements. It works by allowing you to modify the
 * residual and jacobian entries on the secondary nodes and the primary elements.
 */
class ADNodeElemConstraint : public NodeElemConstraintBase
{
public:
  static InputParameters validParams();

  ADNodeElemConstraint(const InputParameters & parameters);

  /// Computes the Nodal residual.
  virtual void computeResidual() override;

  /// Computes the jacobian for the current element.
  virtual void computeJacobian() override;

protected:
  /// prepare the _secondary_to_primary_map (see NodeElemConstraintBase)
  virtual void prepareSecondaryToPrimaryMap() = 0;

  /// This is the virtual method that derived classes should override for computing the residual.
  virtual ADReal computeQpResidual(Moose::ConstraintType type) = 0;

  /// This is the virtual method that derived classes should override for computing the Jacobian.
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType /*type*/);

  /// This is the virtual method that derived classes should override for computing the off-diag Jacobian.
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                        unsigned int /*jvar*/);

  /// Holds the current solution at the current quadrature point
  const ADVariableValue & _u_primary;
  /// Value of the unknown variable on the secondary node
  const ADVariableValue & _u_secondary;

  /// Data members for holding residuals
  ADReal _r;
  std::vector<Real> _residuals;
};
