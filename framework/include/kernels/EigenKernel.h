//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EIGENKERNEL_H
#define EIGENKERNEL_H

#include "KernelBase.h"

// Forward Declarations
class EigenKernel;
class MooseEigenSystem;

template <>
InputParameters validParams<EigenKernel>();

/**
 * The behavior of this kernel is controlled by one problem-wise global parameter
 *    eigen_on_current - bool, to indicate if this kernel is operating on the current solution or
 * old solution
 * This kernel also obtain the postprocessor for eigenvalue by one problem-wise global parameter
 *    eigen_postprocessor - string, the name of the postprocessor to obtain the eigenvalue
 */
class EigenKernel : public KernelBase
{
public:
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int /*jvar*/) override;
  virtual void computeOffDiagJacobianScalar(unsigned int /*jvar*/) override {}

  EigenKernel(const InputParameters & parameters);
  virtual bool enabled() const override;

protected:
  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian() { return 0; }
  virtual Real computeQpOffDiagJacobian(unsigned int /*jvar*/) { return 0; }

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// flag for as an eigen kernel or a normal kernel
  bool _eigen;

  /// EigenKernel always lives in EigenSystem
  MooseEigenSystem * _eigen_sys;

  /**
   * A pointer to the eigenvalue that is stored in a postprocessor
   * This is a pointer so that the method for retrieval (old vs current) may be changed.
   */
  const Real * _eigenvalue;
};

#endif // EIGENKERNEL_H
