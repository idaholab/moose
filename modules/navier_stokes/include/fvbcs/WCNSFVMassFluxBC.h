//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVFluxBCBase.h"

/**
 * Flux boundary condition for the weakly compressible mass conservation equation
 */
class WCNSFVMassFluxBC : public WCNSFVFluxBCBase
{
public:
  static InputParameters validParams();
  WCNSFVMassFluxBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;
};
