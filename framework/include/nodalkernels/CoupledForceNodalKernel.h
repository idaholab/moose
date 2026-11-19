//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericNodalKernel.h"

/**
 * Adds a force proportional to the value of the coupled variable
 */
template <bool is_ad>
class CoupledForceNodalKernelTempl : public GenericNodalKernel<is_ad>
{
public:
  static InputParameters validParams();

  CoupledForceNodalKernelTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  usingGenericNodalKernelMembers;

private:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  const GenericVariableValue<is_ad> & _v;

  /// A multiplicative factor for computing the coupled force
  const Real _coef;
};

typedef CoupledForceNodalKernelTempl<false> CoupledForceNodalKernel;
typedef CoupledForceNodalKernelTempl<true> ADCoupledForceNodalKernel;
