//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVTKEWallFunctionBC.h"
#include "NS.h"
#include "NSEnums.h"

registerADMooseObject("NavierStokesApp", NSFVTKEWallFunctionBC);

InputParameters
NSFVTKEWallFunctionBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addParam<MooseFunctorName>(NS::porosity, 1.0, "Coupled porosity.");
  params.addClassDescription("Neuman boundary condition for the turbulent kinetic energy.");
  return params;
}

NSFVTKEWallFunctionBC::NSFVTKEWallFunctionBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _eps(getFunctor<ADReal>(NS::porosity))
{
}

ADReal
NSFVTKEWallFunctionBC::computeQpResidual()
{
  // return - _eps[_qp] * 0.0;
  return 0.0;
}