//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVExtrapolatedPressureBC.h"

registerMooseObject("NavierStokesApp", LinearFVExtrapolatedPressureBC);

InputParameters
LinearFVExtrapolatedPressureBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionExtrapolatedBC::validParams();
  params.addClassDescription("Adds an extrapolated pressure boundary condition.");
  return params;
}

LinearFVExtrapolatedPressureBC::LinearFVExtrapolatedPressureBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionExtrapolatedBC(parameters)
{
}

Real
LinearFVExtrapolatedPressureBC::computeBoundaryGradientMatrixContribution() const
{
  return 0;
}

Real
LinearFVExtrapolatedPressureBC::computeBoundaryGradientRHSContribution() const
{
  return 0;
}
