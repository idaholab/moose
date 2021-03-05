//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVSolidFluidConvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVSolidFluidConvection);

InputParameters
NSFVSolidFluidConvection::validParams()
{
  auto params = NSFVFluidSolidConvection::validParams();
  params.addClassDescription("Interphase convective heat transfer $\\alpha(T_s-T_f)$ "
                             "in the solid energy conservation equation.");
  return params;
}

NSFVSolidFluidConvection::NSFVSolidFluidConvection(const InputParameters & parameters)
  : NSFVFluidSolidConvection(parameters)
{
}

ADReal
NSFVSolidFluidConvection::computeQpResidual()
{
  return -NSFVFluidSolidConvection::computeQpResidual();
}
