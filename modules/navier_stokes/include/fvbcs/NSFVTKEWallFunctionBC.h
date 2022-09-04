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
 * This boundary condition sets a Neuman boundary condition for the turbulent kinetic energy
 * The user could be using a standard Neumann boundary condition but we create this obeject for consistency
 */
class NSFVTKEWallFunctionBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  NSFVTKEWallFunctionBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Porosity
  const Moose::Functor<ADReal> & _eps;
};