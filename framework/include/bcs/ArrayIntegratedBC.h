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
#include "MooseVariableScalar.h"

/**
 * Base class for deriving any boundary condition of a integrated type
 */
class ArrayIntegratedBC : public IntegratedBCBase, public MooseVariableInterface<RealEigenVector>
{
public:
  static InputParameters validParams();

  ArrayIntegratedBC(const InputParameters & parameters);

  virtual const ArrayMooseVariable & variable() const override { return _var; }

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
   * Method for computing the residual at quadrature points, to be filled in \p residual.
   */
  virtual void computeQpResidual(RealEigenVector & residual) = 0;

  /**
   * Method for computing the diagonal Jacobian at quadrature points
   */
  virtual RealEigenVector computeQpJacobian();

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFEBase & jvar);

  /**
   * This is the virtual that derived classes should override for computing a full Jacobian
   * component
   */
  virtual RealEigenMatrix computeQpOffDiagJacobianScalar(const MooseVariableScalar & jvar);

  /**
   * Put necessary evaluations depending on qp but independent on test functions here
   */
  virtual void initQpResidual() {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here
   */
  virtual void initQpJacobian() {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here for
   * off-diagonal Jacobian assembly
   */
  virtual void initQpOffDiagJacobian(const MooseVariableFEBase &) {}

  ArrayMooseVariable & _var;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;

  /// shape function values (in QPs)
  const ArrayVariablePhiValue & _phi;

  /// test function values (in QPs)
  const ArrayVariableTestValue & _test;

  /// the values of the unknown variable this BC is acting on
  const ArrayVariableValue & _u;

  /// Number of components of the array variable
  const unsigned int _count;

private:
  /// Work vector for residual
  RealEigenVector _work_vector;
};
