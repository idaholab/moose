//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSMOMENTUMINVISCIDSPECIFIEDNORMALFLOWBC_H
#define NSMOMENTUMINVISCIDSPECIFIEDNORMALFLOWBC_H

#include "NSMomentumInviscidBC.h"

// Forward Declarations
class NSMomentumInviscidSpecifiedNormalFlowBC;

template <>
InputParameters validParams<NSMomentumInviscidSpecifiedNormalFlowBC>();

/**
 * Momentum equation boundary condition in which pressure is specified (given)
 * and the value of the convective part is allowed to vary (is computed implicitly).
 */
class NSMomentumInviscidSpecifiedNormalFlowBC : public NSMomentumInviscidBC
{
public:
  NSMomentumInviscidSpecifiedNormalFlowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  const VariableValue & _pressure;
  const Real _rhou_udotn;
};

#endif // NSMOMENTUMINVISCIDSPECIFIEDPRESSUREBC_H
