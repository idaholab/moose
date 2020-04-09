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

// Forward Declaration

/**
 * Calculate heat or mass transfer from a coupled variable to u.
 * This can be used to transfer from a variable specified in a highly conductive domain to a low
 * conductive domain. With this kernel the retardation effect described in the porous flow
 * documentation can be overcome.
 * Alternatively, this kernel can also be used to model simplified precipitation/dissolution,
 * where the Variable is the concentration.
 */
class PorousFlowHeatMassTransfer : public Kernel
{
public:
  static InputParameters validParams();

  PorousFlowHeatMassTransfer(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  virtual Real jac(unsigned int jvar) const;

private:
  const unsigned int _v_var;
  const VariableValue & _v;
  const VariableValue & _coef_var;
};
