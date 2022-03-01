//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class MooseEigenSystem;

/**
 * The behavior of this kernel is controlled by one problem-wise global parameter
 *    eigen_on_current - bool, to indicate if this kernel is operating on the current solution or
 * old solution
 * This kernel also obtain the postprocessor for eigenvalue by one problem-wise global parameter
 *    eigen_postprocessor - string, the name of the postprocessor to obtain the eigenvalue
 */
class EigenKernel : public Kernel
{
public:
  static InputParameters validParams();

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int /*jvar*/) override {}

  EigenKernel(const InputParameters & parameters);
  virtual bool enabled() const override;

protected:
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
