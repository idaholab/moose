//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBCBase.h"
#include "MooseVariableInterface.h"

/**
 * Base class for deriving any boundary condition of a integrated type
 */
class VectorIntegratedBC : public IntegratedBCBase, public MooseVariableInterface<RealVectorValue>
{
public:
  static InputParameters validParams();

  VectorIntegratedBC(const InputParameters & parameters);

  virtual const VectorMooseVariable & variable() const override { return _var; }

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  /**
   * Computes d-ivar-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  /**
   * Method for computing the residual at quadrature points
   */
  virtual Real computeQpResidual() = 0;

  /**
   * Method for computing the diagonal Jacobian at quadrature points
   */
  virtual Real computeQpJacobian() { return 0; }

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int /*jvar*/) { return 0; }

  /**
   * Method for computing an off-diagonal jacobian component from a scalar var.
   */
  virtual Real computeQpOffDiagJacobianScalar(unsigned int /*jvar*/) { return 0; }

  VectorMooseVariable & _var;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;

  // shape functions

  /// shape function values (in QPs)
  const VectorVariablePhiValue & _phi;

  // test functions

  /// test function values (in QPs)
  const VectorVariableTestValue & _test;

  // solution variable

  /// the values of the unknown variable this BC is acting on
  const VectorVariableValue & _u;
};
