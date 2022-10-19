//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"

/**
 * Implements a source term proportional to the value of a coupled variable. Weak form: $(\\psi_i,
 * -\\sigma v)$.
 */
template <bool is_ad>
class CoupledForceTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  CoupledForceTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  usingGenericKernelMembers;

private:
  /// Coupled variable number
  unsigned int _v_var;
  /// Coupled variable
  const GenericVariableValue<is_ad> & _v;
  /// Multiplier for the coupled force term
  Real _coef;
};

typedef CoupledForceTempl<false> CoupledForce;
typedef CoupledForceTempl<true> ADCoupledForce;
