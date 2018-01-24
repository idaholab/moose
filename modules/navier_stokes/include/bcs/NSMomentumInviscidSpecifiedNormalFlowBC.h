/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
