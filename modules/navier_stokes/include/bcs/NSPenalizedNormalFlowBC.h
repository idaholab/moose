//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSIntegratedBC.h"

// Forward Declarations

/**
 * This class penalizes the the value of u.n on the boundary
 * so that it matches some desired value.
 */
class NSPenalizedNormalFlowBC : public NSIntegratedBC
{
public:
  static InputParameters validParams();

  NSPenalizedNormalFlowBC(const InputParameters & parameters);

protected:
  /**
   * The standard interface functions.
   */
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  const Real _penalty;
  const Real _specified_udotn;
};
