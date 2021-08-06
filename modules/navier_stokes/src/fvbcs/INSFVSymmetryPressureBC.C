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
registerMooseObject("NavierStokesApp", INSFVSymmetryScalarBC);

InputParameters
INSFVSymmetryPressureBC::validParams()
{
  auto params = FVFluxBC::validParams();
  params += INSFVSymmetryBC::validParams();
  params.addClassDescription("Though not applied to velocity, this object ensures that the "
                             "flux (velocity times the advected quantity) into a "
                             "symmetry boundary is zero. When applied to pressure for the mass "
                             "equation, this makes the normal velocity zero since density is "
                             "constant");
  return params;
}

INSFVSymmetryPressureBC::INSFVSymmetryPressureBC(const InputParameters & params)
  : FVFluxBC(params), INSFVSymmetryBC(params)
{
}

ADReal
INSFVSymmetryPressureBC::computeQpResidual()
{
  return 0;
}
