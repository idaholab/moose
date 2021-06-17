//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVSymmetryPressureBC.h"
#include "InputParameters.h"

registerMooseObject("NavierStokesApp", INSFVSymmetryPressureBC);

InputParameters
INSFVSymmetryPressureBC::validParams()
{
  auto params = INSFVSymmetryBC::validParams();
  params.addClassDescription("Though applied to the pressure, this object ensures that the "
                             "velocity perpendicular to a symmetry bounadry is zero by setting the "
                             "mass flow rate across the symmetry boundary to zero.");
  return params;
}

INSFVSymmetryPressureBC::INSFVSymmetryPressureBC(const InputParameters & params)
  : INSFVSymmetryBC(params)
{
}

ADReal
INSFVSymmetryPressureBC::computeQpResidual()
{
  return 0;
}
