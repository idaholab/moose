//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianEnergyFluxBC.h"
#include "Function.h"

registerMooseObject("HeatConductionApp", GaussianEnergyFluxBC);

InputParameters
GaussianEnergyFluxBC::beamParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<Real>("P0", "The total power of the beam.");
  params.addRequiredParam<Real>(
      "R", "The radius at which the beam intensity falls to $1/e^2$ of its axis value.");
  auto beam_coord_doc = [](const std::string & coord)
  {
    return "The " + coord +
           " coordinate of the center of the beam as a function of time. Note that we will pass "
           "the origin as the spatial argument to the function; any spatial dependence in the "
           "passed-in function will be ignored";
  };
  params.addParam<FunctionName>("x_beam_coord", 0, beam_coord_doc("x"));
  params.addParam<FunctionName>("y_beam_coord", 0, beam_coord_doc("y"));
  params.addParam<FunctionName>("z_beam_coord", 0, beam_coord_doc("z"));
  params.addClassDescription("Describes an incoming heat flux beam with a Gaussian profile");
  return params;
}

InputParameters
GaussianEnergyFluxBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params += GaussianEnergyFluxBC::beamParams();
  return params;
}

GaussianEnergyFluxBC::GaussianEnergyFluxBC(const InputParameters & params)
  : ADIntegratedBC(params),
    _P0(getParam<Real>("P0")),
    _R(getParam<Real>("R")),
    _x_beam_coord(getFunction("x_beam_coord")),
    _y_beam_coord(getFunction("y_beam_coord")),
    _z_beam_coord(getFunction("z_beam_coord"))
{
}

ADReal
GaussianEnergyFluxBC::computeQpResidual()
{
  return _test[_i][_qp] * beamFlux(*this, _ad_q_points[_qp]);
}
