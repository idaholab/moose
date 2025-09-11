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

protected:
  /**
   * The user can override this function to compute the residual at a node.
   */
  virtual void computeQpResidual(RealEigenVector & residual) = 0;

  /**
   * The user can override this function to compute the "on-diagonal"
   * Jacobian contribution.  If not overriden,
   * returns an array of 1s.
   */
  virtual RealEigenVector computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for
   * computing an off-diagonal jacobian component.
   */
  virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFieldBase & jvar);

  /// variable this works on
  MooseVariableFE<RealEigenVector> & _var;

  /// Value of the unknown variable this is acting on
  const ArrayVariableValue & _u;

  /// Number of components of the array variable
  const unsigned int _count;

private:
  /**
   * Prepare our array ivar degrees of freedom
   */
  void prepareDofs();

  /// Work vector for residual
  RealEigenVector _work_vector;

  /// Work vector for the degree of freedom indices
  std::vector<dof_id_type> _work_dofs;
};
