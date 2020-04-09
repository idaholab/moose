//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSMomentumInviscidBC.h"

// Forward Declarations

/**
 * Momentum equation boundary condition in which pressure is specified (given)
 * and the value of the convective part is allowed to vary (is computed implicitly).
 */
class NSMomentumInviscidSpecifiedPressureBC : public NSMomentumInviscidBC
{
public:
  static InputParameters validParams();

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
