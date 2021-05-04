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
/**
 * The DGKernel class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces) along with its associated lower-d elements.
 */
class DGLowerDKernel : public DGKernel
{
public:
  /**
   * Factory constructor initializes all internal references needed for residual computation.
   *
   *
   * @param parameters The parameters object for holding additional parameters for kernels and
   * derived kernels
   */
  static InputParameters validParams();

  DGLowerDKernel(const InputParameters & parameters);

  /**
   * The variable that this kernel operates on.
   */
  const MooseVariableFEBase & variable() const override final { return _var; }

  /**
   * The variable that this kernel operates on.
   */
  const MooseVariableFEBase & lowerDVariable() const { return _lowerd_var; }

  /**
   * Computes the residual for this element, the neighbor and the lower-d element
   */
  virtual void computeResidual() override;

  /**
   * Computes the nine pieces of element/neighbor/lower-d - element/neighbor/lower-d Jacobian
   */
  virtual void computeJacobian() override;

  /**
   * Computes d-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  /**
   * Computes the Lower part of residual for the variable on the lower-d element
   */
  virtual void computeLowerDResidual();

  /**
   * Method for computing the Lower part of residual at quadrature points, to be filled in \p
   * residual
   */
  virtual Real computeLowerDQpResidual() = 0;

  /**
   * Computes one of the five pieces of Jacobian involving lower-d
   */
  virtual void computeLowerDJacobian(Moose::ConstraintJacobianType jacobian_type);

  /**
   * Computes one of the five pieces of Jacobian involving lower-d at quadrature points
   */
  virtual Real computeLowerDQpJacobian(Moose::ConstraintJacobianType jacobian_type) = 0;

  /**
   * Computes one of the five pieces of off-diagonal Jacobian involving lower-d
   */
  virtual void computeOffDiagLowerDJacobian(Moose::ConstraintJacobianType type,
                                            const MooseVariableFEBase & jvar);

  /**
   * Computes one of the five pieces of off-diagonal Jacobian involving lower-d at quadrature points
   */
  virtual Real computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                              const MooseVariableFEBase & /*jvar*/)
  {
    return 0;
  }

  /**
   * Put necessary evaluations depending on qp but independent on test functions here
   */
  virtual void initLowerDQpResidual() {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here
   */
  virtual void initLowerDQpJacobian(Moose::ConstraintJacobianType) {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here for
   * off-diagonal Jacobian assembly
   */
  virtual void initLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType,
                                           const MooseVariableFEBase &)
  {
  }

  /// Variable this kernel operates on
  const MooseVariable & _lowerd_var;
  /// Holds the current solution at the current quadrature point on the face.
  const VariableValue & _lambda;
  /// Shape functions
  const VariablePhiValue & _phi_lambda;
  /// test functions
  const VariableTestValue & _test_lambda;
};
