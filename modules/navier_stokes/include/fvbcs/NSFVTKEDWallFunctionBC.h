//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"
#include "NSEnums.h"

/**
 * This boundary condition sets a Robin boundary condition for the turbulent kinetic energy dissipation
 * The user could be using a standard Robin boundary condition but this object is more practical and less subjected to errors
 */
class NSFVTKEDWallFunctionBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  NSFVTKEDWallFunctionBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Porosity
  const Moose::Functor<ADReal> & _eps;

  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;

  /// Density
  const Moose::Functor<ADReal> & _rho;
  /// Turbuelnt viscosity
  const Moose::Functor<ADReal> & _mu_t;
  /// C-mu closure coefficient
  const Moose::Functor<ADReal> & _C_mu;
};