//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayIntegratedBC.h"

/**
 * Base class for deriving any boundary condition of a integrated type
 */
class ArrayLowerDIntegratedBC : public ArrayIntegratedBC
{
public:
  static InputParameters validParams();

  ArrayLowerDIntegratedBC(const InputParameters & parameters);

  const ArrayMooseVariable & variable() const override final { return _var; }
  const ArrayMooseVariable & lowerDVariable() const { return _lowerd_var; }

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  /**
   * Method for computing the Lower part of residual at quadrature points, to be filled in \p
   * residual.
   */
  virtual void computeLowerDQpResidual(RealEigenVector & residual) = 0;

  /**
   * Method for computing the LowerLower, PrimaryLower and LowerPrimary parts of Jacobian
   */
  virtual void computeLowerDJacobian(Moose::ConstraintJacobianType type);

  /**
   * Method for computing the LowerLower, PrimaryLower and LowerPrimary parts of Jacobian at
   * quadrature points
   */
  virtual RealEigenVector computeLowerDQpJacobian(Moose::ConstraintJacobianType) = 0;

  /**
   * Method for computing an off-diagonal jacobian component
   */
  void computeLowerDOffDiagJacobian(Moose::ConstraintJacobianType type,
                                    const unsigned int jvar_num);

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual RealEigenMatrix computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                         const MooseVariableFEBase & jvar);

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
  const ArrayMooseVariable & _lowerd_var;
  /// Holds the current solution at the current quadrature point on the face.
  const ArrayVariableValue & _lambda;
  /// Shape functions
  const ArrayVariablePhiValue & _phi_lambda;
  /// test functions
  const ArrayVariableTestValue & _test_lambda;

private:
  /// Work vector for residual
  RealEigenVector _work_vector;
};
