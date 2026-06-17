//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"

/**
 * Base class for PorousFlow kernels that use mass-lumped (nodal) material properties.
 *
 * These kernels index material properties at [_i] (the test-function node) rather than [_qp].
 * That breaks the default ADKernel Jacobian assembly, which assumes every test function's residual
 * depends on the same set of DOFs; see computeJacobian() in PorousFlowLumpedKernelBase.C for the
 * full explanation.  This base class overrides the three Jacobian methods for the AD path to use
 * addJacobianWithoutConstraints, which reads each row's column set from its own residual.
 */
template <bool is_ad>
class PorousFlowLumpedKernelBaseTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();
  PorousFlowLumpedKernelBaseTempl(const InputParameters & parameters);

protected:
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void jacobianSetup() override;

  usingGenericKernelMembers;

private:
  // Cache to avoid recomputing AD residuals for each off-diagonal jvar on the same element,
  // mirroring the _my_elem guard in ADKernel::computeOffDiagJacobian.
  const Elem * _my_elem_lma = nullptr;
};

typedef PorousFlowLumpedKernelBaseTempl<false> PorousFlowLumpedKernelBase;
typedef PorousFlowLumpedKernelBaseTempl<true> ADPorousFlowLumpedKernelBase;
