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

InputParameters
GrayLambertNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the GrayLambertSurfaceRadiationBase UO");
  params.addParam<bool>(
      "reconstruct_emission",
      true,
      "Flag to apply constant heat flux on sideset or reconstruct emission by T^4 law.");
  params.addClassDescription("This BC imposes a heat flux density that is computed from the "
                             "GrayLambertSurfaceRadiationBase userobject.");
  return params;
}

GrayLambertNeumannBC::GrayLambertNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _glsr_uo(getUserObject<GrayLambertSurfaceRadiationBase>("surface_radiation_object_name")),
    _reconstruct_emission(getParam<bool>("reconstruct_emission"))
{
}

Real
GrayLambertNeumannBC::computeQpResidual()
{
  if (!_reconstruct_emission)
    return _test[_i][_qp] * _glsr_uo.getSurfaceHeatFluxDensity(_current_boundary_id);

  Real eps = _glsr_uo.getSurfaceEmissivity(_current_boundary_id);
  Real emission = _sigma_stefan_boltzmann * MathUtils::pow(_u[_qp], 4);
  return _test[_i][_qp] * eps * (emission - _glsr_uo.getSurfaceIrradiation(_current_boundary_id));
}

Real
GrayLambertNeumannBC::computeQpJacobian()
{
  // this is not the exact Jacobian but it ensures correct scaling
  return _test[_i][_qp] * _sigma_stefan_boltzmann *
         _glsr_uo.getSurfaceEmissivity(_current_boundary_id) * 4 * MathUtils::pow(_u[_qp], 3) *
         _phi[_j][_qp];
}
