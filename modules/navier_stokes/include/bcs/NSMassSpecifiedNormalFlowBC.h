//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSMassBC.h"

// Forward Declarations

/**
 * This class implements the mass equation boundary term with
 * a specified value of rho*(u.n) imposed weakly.
 *
 * Note: if you wish to impose rho*(u.n) = 0 weakly, you don't
 * actually need this class, that is the natural boundary condition.
 */
class NSMassSpecifiedNormalFlowBC : public NSMassBC
{
public:
  static InputParameters validParams();

  NSMassSpecifiedNormalFlowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  const Real _rhoun;
};
