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
 * the rho*(u.n) boundary integral computed implicitly.
 */
class NSMassUnspecifiedNormalFlowBC : public NSMassBC
{
public:
  static InputParameters validParams();

  NSMassUnspecifiedNormalFlowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};
