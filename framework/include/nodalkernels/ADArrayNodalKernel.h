//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernelBase.h"
#include "MooseVariableInterface.h"

/**
 * Base class for creating nodal kernels with hand-coded Jacobians
 */
class ADArrayNodalKernel : public NodalKernelBase, public MooseVariableInterface<RealEigenVector>
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  static InputParameters validParams();

  ADArrayNodalKernel(const InputParameters & parameters);

  /**
   * Compute the residual at the current node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpResidual()
   */
  virtual void computeResidual() override;

  /**
   * Compute the Jacobian at one node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpJacobian()
   */
  virtual void computeJacobian() override;

  /**
   * Compute the off-diagonal Jacobian at one node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpOffDiagJacobian()
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /**
   * Gets the variable this is active on
   * @return the variable
   */
  const MooseVariableFE<RealEigenVector> & variable() const override { return _var; }

  virtual void jacobianSetup() override { _my_node = nullptr; }

protected:
  /**
   * The user can override this function to compute the residual at a node.
   */
  virtual void computeQpResidual(ADRealEigenVector & residual) = 0;

  /**
   * Dummy method so we can make derived generic classes that template on <bool is_ad> */
  virtual void computeQpJacobian()
  {
    mooseError("I'm an AD object, so computeQpJacobian should never be called");
  }

  void setJacobian(unsigned int, unsigned int, Real)
  {
    mooseError("I'm an AD object, so setJacobian should never be called");
  }

  /// variable this works on
  MooseVariableFE<RealEigenVector> & _var;

  /// Value of the unknown variable this is acting on
  const ADArrayVariableValue & _u;

  /// Number of components of the array variable
  const unsigned int _count;

private:
  /// Work vector for residual
  ADRealEigenVector _work_vector;

  /// Cache variable to make sure we don't do duplicate AD computations
  const Node * _my_node = nullptr;
};
