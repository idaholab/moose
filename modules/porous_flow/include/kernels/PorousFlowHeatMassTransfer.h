//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWHEATMASSTRANSFER_H
#define POROUSFLOWHEATMASSTRANSFER_H

#include "Kernel.h"

// Forward Declaration
class PorousFlowHeatMassTransfer;

template <>
InputParameters validParams<PorousFlowHeatMassTransfer>();

/**
 * Calculate heat or mass transfer from a coupled variable to u.
 * This can be used to transfer from a variable specified in a highly conductive domain to a low
 * conductive domain. With this kernel the retardation effect describted in the porous flow
 * documentation can be overcome.
 */
class PorousFlowHeatMassTransfer : public Kernel
{
public:
  PorousFlowHeatMassTransfer(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  virtual Real jac(unsigned int jvar) const;

private:
  unsigned int _v_var;
  const VariableValue & _v;
  Real _coef;
};

#endif // POROUSFLOWHEATMASSTRANSFER_H
