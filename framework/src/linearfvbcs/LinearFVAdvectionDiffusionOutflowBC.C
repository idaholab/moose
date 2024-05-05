//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionOutflowBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionOutflowBC);

InputParameters
LinearFVAdvectionDiffusionOutflowBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionExtrapolatedBC::validParams();
  params.addClassDescription("Adds a boundary condition which represents a surface with outflowing "
                             "material with a constant velocity. This kernel is only compatible "
                             "with advection-diffusion problems.");
  return params;
}

LinearFVAdvectionDiffusionOutflowBC::LinearFVAdvectionDiffusionOutflowBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionExtrapolatedBC(parameters)
{
}

Real
LinearFVAdvectionDiffusionOutflowBC::computeBoundaryNormalGradient() const
{
  return 0;
}

Real
LinearFVAdvectionDiffusionOutflowBC::computeBoundaryGradientMatrixContribution() const
{
  return 0;
}

Real
LinearFVAdvectionDiffusionOutflowBC::computeBoundaryGradientRHSContribution() const
{
  return 0;
}
