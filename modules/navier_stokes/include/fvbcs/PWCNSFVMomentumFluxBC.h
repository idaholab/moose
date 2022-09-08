//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVMomentumFluxBC.h"

/**
 * Flux boundary conditions for the porous weakly compressible momentum equation
 */
class PWCNSFVMomentumFluxBC : public WCNSFVMomentumFluxBC
{
public:
  static InputParameters validParams();
  PWCNSFVMomentumFluxBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// Porosity functor
  const Moose::Functor<ADReal> & _eps;
};
