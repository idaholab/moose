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
GaussianEnergyFluxBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription("Describes an incoming heat flux beam with a Gaussian profile");
  params.addRequiredParam<Real>("P0", "The total power of the beam.");
  params.addRequiredParam<Real>(
      "R", "The radius at which the beam intensity falls to $1/e^2$ of its axis value.");
  params.addParam<FunctionName>("x_beam_coord", 0, "The x coordinate of the center of the beam");
  params.addParam<FunctionName>("y_beam_coord", 0, "The y coordinate of the center of the beam");
  params.addParam<FunctionName>("z_beam_coord", 0, "The z coordinate of the center of the beam");
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
  const RealVectorValue beam_coords{_x_beam_coord.value(_t, _q_point[_qp]),
                                    _y_beam_coord.value(_t, _q_point[_qp]),
                                    _z_beam_coord.value(_t, _q_point[_qp])};
  const auto r = (_ad_q_points[_qp] - beam_coords).norm();
  return -_test[_i][_qp] * 2 * _P0 / (libMesh::pi * _R * _R) * std::exp(-2 * r * r / (_R * _R));
}
