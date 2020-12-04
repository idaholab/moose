//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumAdvectionFunctionBC.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

/**
 * Implements the mass equation advection term on boundaries. Only useful
 * for MMS since it requires exact solution information
 */
class INSFVMassAdvectionFunctionBC : public INSFVMomentumAdvectionFunctionBC
{
public:
  static InputParameters validParams();
  INSFVMassAdvectionFunctionBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;
};

#endif
