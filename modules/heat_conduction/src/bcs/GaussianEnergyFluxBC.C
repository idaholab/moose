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
  params.addRequiredParam<Real>("reff",
                                "The effective radius describing the radial distribution of the "
                                "beam energy. This should be non-dimensional.");
  params.addRequiredParam<Real>("F0", "The average heat flux of the beam.");
  params.addRequiredParam<Real>("R", "The beam radius");
  params.addParam<FunctionName>("x_beam_coord", 0, "The x coordinate of the center of the beam");
  params.addParam<FunctionName>("y_beam_coord", 0, "The y coordinate of the center of the beam");
  params.addParam<FunctionName>("z_beam_coord", 0, "The z coordinate of the center of the beam");
  return params;
}

GaussianEnergyFluxBC::GaussianEnergyFluxBC(const InputParameters & params)
  : ADIntegratedBC(params),
    _reff(getParam<Real>("reff")),
    _F0(getParam<Real>("F0")),
    _R(getParam<Real>("R")),
    _x_beam_coord(getFunction("x_beam_coord")),
    _y_beam_coord(getFunction("y_beam_coord")),
    _z_beam_coord(getFunction("z_beam_coord"))
{
}

ADReal
GaussianEnergyFluxBC::computeQpResidual()
{
  RealVectorValue beam_coords{_x_beam_coord.value(_t, _q_point[_qp]),
                              _y_beam_coord.value(_t, _q_point[_qp]),
                              _z_beam_coord.value(_t, _q_point[_qp])};
  auto r = (_ad_q_points[_qp] - beam_coords).norm();
  return -_test[_i][_qp] * 2. * _reff * _F0 * std::exp(-_reff * r * r / (_R * _R));
}
