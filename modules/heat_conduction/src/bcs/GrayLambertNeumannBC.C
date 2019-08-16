//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrayLambertNeumannBC.h"
#include "MathUtils.h"

registerMooseObject("HeatConductionApp", GrayLambertNeumannBC);

Real GrayLambertNeumannBC::_sigma_stefan_boltzmann = 5.670367e-8;

template <>
InputParameters
validParams<GrayLambertNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the GrayLambertSurfaceRadiation UO");
  params.addClassDescription("This BC imposes a heat flux density that is computed from the "
                             "GrayLambertSurfaceRadiation userobject.");
  return params;
}

GrayLambertNeumannBC::GrayLambertNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _glsr_uo(getUserObject<GrayLambertSurfaceRadiation>("surface_radiation_object_name"))
{
}

Real
GrayLambertNeumannBC::computeQpResidual()
{
  return _test[_i][_qp] * _glsr_uo.getSurfaceHeatFluxDensity(_current_boundary_id);
}

Real
GrayLambertNeumannBC::computeQpJacobian()
{
  // this is not the exact Jacobian but it ensures correct scaling
  return _test[_i][_qp] * _sigma_stefan_boltzmann * 4 * MathUtils::pow(_u[_qp], 3) * _phi[_j][_qp];
}
