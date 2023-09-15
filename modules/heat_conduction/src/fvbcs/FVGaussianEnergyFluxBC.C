//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVGaussianEnergyFluxBC.h"
#include "GaussianEnergyFluxBC.h"
#include "Function.h"

registerMooseObject("HeatConductionApp", FVGaussianEnergyFluxBC);

InputParameters
FVGaussianEnergyFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params += GaussianEnergyFluxBC::beamParams();
  return params;
}

FVGaussianEnergyFluxBC::FVGaussianEnergyFluxBC(const InputParameters & params)
  : FVFluxBC(params),
    _P0(getParam<Real>("P0")),
    _R(getParam<Real>("R")),
    _x_beam_coord(getFunction("x_beam_coord")),
    _y_beam_coord(getFunction("y_beam_coord")),
    _z_beam_coord(getFunction("z_beam_coord"))
{
}

ADReal
FVGaussianEnergyFluxBC::computeQpResidual()
{
  return GaussianEnergyFluxBC::beamFlux(*this, _face_info->faceCentroid());
}
