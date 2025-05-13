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
 * A ADNodeElemConstraint is used when you need to create constraints between
 * a secondary node and a primary element. It works by allowing you to modify the
 * residual and jacobian entries on the secondary node and the primary element.
 */
class ADNodeElemConstraint : public NodeElemConstraintBase
{
public:
  static InputParameters validParams();

  ADNodeElemConstraint(const InputParameters & parameters);

  /// Computes the residual Nodal residual.
  virtual void computeResidual() override;

  /// Computes the jacobian for the current element.
  virtual void computeJacobian() override;

protected:
  /// prepare the _secondary_to_primary_map
  virtual void prepareSecondaryToPrimaryMap() = 0;

  /// This is the virtual that derived classes should override for computing the residual.
  virtual ADReal computeQpResidual(Moose::ConstraintType type) = 0;

  /// This is the virtual that derived classes should override for computing the Jacobian.
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType /*type*/) { return 0; }

  /// This is the virtual that derived classes should override for computing the off-diag Jacobian.
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                        unsigned int /*jvar*/)
  {
    return 0;
  }

  /// Holds the current solution at the current quadrature point
  const ADVariableValue & _u_primary;
  /// Value of the unknown variable on the secondary node
  const ADVariableValue & _u_secondary;

  /// Data members for holding residuals
  ADReal _r;
  std::vector<Real> _residuals;
};
