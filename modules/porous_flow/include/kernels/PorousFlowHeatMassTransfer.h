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

// Forward Declaration

/**
 * Calculate heat or mass transfer from a coupled variable to u.
 * This can be used to transfer from a variable specified in a highly conductive domain to a low
 * conductive domain. With this kernel the retardation effect described in the porous flow
 * documentation can be overcome.
 * Alternatively, this kernel can also be used to model simplified precipitation/dissolution,
 * where the Variable is the concentration.
 *
 * Templated on is_ad: the false instantiation uses the hand-coded Jacobian;
 * the true instantiation propagates derivatives through the AD residual.
 */
template <bool is_ad>
class PorousFlowHeatMassTransferTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowHeatMassTransferTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  virtual Real jac(unsigned int jvar) const;

  usingGenericKernelMembers;

private:
  const unsigned int _v_var;
  const GenericVariableValue<is_ad> & _v;
  const VariableValue & _coef_var;
};

typedef PorousFlowHeatMassTransferTempl<false> PorousFlowHeatMassTransfer;
typedef PorousFlowHeatMassTransferTempl<true> ADPorousFlowHeatMassTransfer;
