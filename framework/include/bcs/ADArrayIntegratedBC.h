//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBCBase.h"
#include "MooseVariableInterface.h"
#include "ADFunctorInterface.h"

/**
 * Base class for integrated boundary conditions on array variables whose Jacobians are computed
 * with automatic differentiation
 */
class ADArrayIntegratedBC : public IntegratedBCBase,
                            public MooseVariableInterface<RealEigenVector>,
                            public ADFunctorInterface
{
public:
  static InputParameters validParams();

  ADArrayIntegratedBC(const InputParameters & parameters);

  virtual const ArrayMooseVariable & variable() const override { return _var; }

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  /**
   * Compute this boundary condition's contribution to the residual at the current quadrature
   * point, to be filled in \p residual.
   */
  virtual void computeQpResidual(ADRealEigenVector & residual) = 0;

  /**
   * Put necessary evaluations depending on qp but independent on test functions here
   */
  virtual void initQpResidual() {}

  /// The array variable this boundary condition acts on
  ArrayMooseVariable & _var;

  /// normals at quadrature points
  const MooseArray<ADPoint> & _normals;

  /// test function values (in QPs)
  const ArrayVariableTestValue & _test;

  /// gradients of test functions (in QPs)
  const ArrayVariableTestGradient & _grad_test;

  /// the values of the unknown variable this BC is acting on
  const ADArrayVariableValue & _u;

  /// the gradient of the unknown variable this BC is acting on
  const ADArrayVariableGradient & _grad_u;

  /// Number of components of the array variable
  const unsigned int _count;

private:
  /// Work vector for the residual at a quadrature point
  ADRealEigenVector _work_vector;

  /// Local AD residuals for the Jacobian computation
  std::vector<ADReal> _local_ad_re;
};
