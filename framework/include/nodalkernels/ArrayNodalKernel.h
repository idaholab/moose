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
class ArrayNodalKernel : public NodalKernelBase, public MooseVariableInterface<RealEigenVector>
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  static InputParameters validParams();

  ArrayNodalKernel(const InputParameters & parameters);

  /**
   * Compute and assemble the residual at the current node
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpResidual()
   */
  virtual void computeResidual() override;

  /**
   * Compute and assemble the Jacobian at one node.
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
   * @param jvar Coupled variable for the off-diagonal Jacobian contribution
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /**
   * Gets the variable this is active on
   * @return the variable
   */
  const MooseVariableFE<RealEigenVector> & variable() const override { return _var; }

protected:
  /**
   * The user must override this function to compute the residual at a node.
   * @param residual Reference to the residual, computed by this method
   */
  virtual void computeQpResidual(RealEigenVector & residual) = 0;

  /**
   * The user can override this function to compute the intra-variable
   * off-diagonal Jacobian contribution (the coupling between array
   * components of the variable itself). Use setJacobian in this function.
   */
  virtual void computeQpJacobian() {}

  /**
   * The user can override this function to compute the inter-variable
   * off-diagonal Jacobian contribution (the coupling between array
   * components of of the kernel variable and the coupled variable jvar).
   * Use setJacobian in this function.
   */
  virtual void computeQpOffDiagJacobian(unsigned int /*jvar*/) {}

  void setJacobian(unsigned int i, unsigned int j, Real value);

  /// variable this works on
  MooseVariableFE<RealEigenVector> & _var;

  /// Value of the unknown variable this is acting on
  const ArrayVariableValue & _u;

  /// Number of components of the array variable
  const unsigned int _count;

private:
  /// Work vector for residual
  RealEigenVector _work_vector;

  /// DOF indices
  const std::vector<dof_id_type> * _ivar_indices;
  const std::vector<dof_id_type> * _jvar_indices;

  /// scaling factors
  const std::vector<Real> & _scaling;
};
