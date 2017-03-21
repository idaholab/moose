/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMOMENTUMINVISCIDSPECIFIEDPRESSUREBC_H
#define NSMOMENTUMINVISCIDSPECIFIEDPRESSUREBC_H

#include "NSMomentumInviscidBC.h"

// Forward Declarations
class NSMomentumInviscidSpecifiedPressureBC;

template <>
InputParameters validParams<NSMomentumInviscidSpecifiedPressureBC>();

/**
 * Momentum equation boundary condition in which pressure is specified (given)
 * and the value of the convective part is allowed to vary (is computed implicitly).
 */
class NSMomentumInviscidSpecifiedPressureBC : public NSMomentumInviscidBC
{
public:
  NSMomentumInviscidSpecifiedPressureBC(const InputParameters & parameters);

  virtual ~NSMomentumInviscidSpecifiedPressureBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // The specified value of the pressure.  This must be passed to the parent's
  // pressureQpResidualHelper function.
  Real _specified_pressure;
};

#endif // NSMOMENTUMINVISCIDSPECIFIEDPRESSUREBC_H
