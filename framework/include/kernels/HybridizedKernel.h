//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelBase.h"

class HybridizedKernel : public KernelBase
{
public:
  static InputParameters validParams();

  HybridizedKernel(const InputParameters & parameters);

  virtual void computeResidual() override final;
  virtual void computeJacobian() override final;
  virtual void computeOffDiagJacobian(unsigned int) override final;
  virtual void computeOffDiagJacobianScalar(unsigned int) override final;

  virtual void computeResidualAndJacobian() override final;

  /**
   * Here we compute the updates to the primal variables (solution and gradient) now that we have
   * the update to our dual (Lagrange multiplier) variable
   */
  void computePostLinearSolve();

  virtual void initialSetup() override;

  static const std::string lm_increment_vector_name;

protected:
  /**
   * Implement the assembly process
   * @param computing_global_data Whether we are computing global data, e.g. the residual and
   * Jacobian for the the global Lagrange multiplier degrees of freedom
   */
  virtual void assemble(bool computing_global_data) = 0;

  /**
   * The (ghosted) increment of the Lagrange multiplier vector. This will be used post-linear solve
   * (pre linesearch) to update the primal solution which resides in the auxiliary system
   */
  const NumericVector<Number> * _lm_increment;
};

inline HybridizedKernel::HybridizedKernel(const InputParameters & parameters)
  : KernelBase(parameters)
{
}

inline void
HybridizedKernel::computeResidual()
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedKernel::computeJacobian()
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedKernel::computeOffDiagJacobian(unsigned int)
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedKernel::computeOffDiagJacobianScalar(unsigned int)
{
  mooseError("Hybridized kernels only implement computeResidualAndJacobian");
}

inline void
HybridizedKernel::computeResidualAndJacobian()
{
  assemble(true);
}

inline void
HybridizedKernel::computePostLinearSolve()
{
  assemble(false);
}

inline void
HybridizedKernel::initialSetup()
{
  KernelBase::initialSetup();
  _lm_increment = &_sys.getVector(lm_increment_vector_name);
}
