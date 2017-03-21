/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMOMENTUMINVISCIDNOPRESSUREIMPLICITFLOWBC_H
#define NSMOMENTUMINVISCIDNOPRESSUREIMPLICITFLOWBC_H

#include "NSMomentumInviscidBC.h"

// Forward Declarations
class NSMomentumInviscidNoPressureImplicitFlowBC;

template <>
InputParameters validParams<NSMomentumInviscidNoPressureImplicitFlowBC>();

/**
 * Momentum equation boundary condition used when pressure *is not*
 * integrated by parts, i.e. when there is "no pressure" term on the boundary.
 * In this case the flow is not specified either, so that the entire term is
 * handled implicitly.
 */
class NSMomentumInviscidNoPressureImplicitFlowBC : public NSMomentumInviscidBC
{
public:
  NSMomentumInviscidNoPressureImplicitFlowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif // NSMOMENTUMINVISCIDNOPRESSUREIMPLICITFLOWBC_H
