//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADNODEFACECONSTRAINT_H
#define ADNODEFACECONSTRAINT_H

// MOOSE includes
#include "ADNodeFaceConstraintBase.h"

// Forward Declarations
class ADNodeFaceConstraint;

template <>
InputParameters validParams<ADNodeFaceConstraint>();

/**
 * A ADNodeFaceConstraint is used when you need to create constraints between
 * two surfaces in a mesh.  It works by allowing you to modify the residual
 * and jacobian entries on "this" side (the node side, also referred to as
 * the slave side) and the "other" side (the face side, also referred to as
 * the master side)
 *
 * This is common for contact algorithms and other constraints.
 */
class ADNodeFaceConstraint : public ADNodeFaceConstraintBase
{
public:
  ADNodeFaceConstraint(const InputParameters & parameters);
  virtual ~ADNodeFaceConstraint();

  /**
   * Computes the residual Nodal residual.
   */
  virtual void computeResidual() override;

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian() override;

  /**
   * Computes d-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
protected:
  /// Value of the unknown variable this BC is action on
  const VariableValue & _u_slave;
  /// Shape function on the slave side.  This will always
  VariablePhiValue _phi_slave;

  /// Side shape function.
  const VariablePhiValue & _phi_master;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_master;

  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_master;
  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u_master;
};

#endif
