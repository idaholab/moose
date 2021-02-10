//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFluidEnergyAdvectiveFlux.h"
#include "NS.h"
#include "Assembly.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSFluidEnergyAdvectiveFlux);

defineADValidParams(CNSFluidEnergyAdvectiveFlux,
                    AdvectiveFluxKernel,
                    params.addClassDescription("Advective flux $\\nabla\\cdot(\\epsilon H_f\\rho_f\\vec{V})$ "
                      "in the Navier-Stokes and Euler fluid energy conservation equations."););

CNSFluidEnergyAdvectiveFlux::CNSFluidEnergyAdvectiveFlux(
    const InputParameters & parameters)
  : AdvectiveFluxKernel(parameters),
    _specific_total_enthalpy(getADMaterialProperty<Real>(nms::specific_total_enthalpy)),
    _momentum(getADMaterialProperty<RealVectorValue>(nms::momentum)),
    _grad_rho_et(getADMaterialProperty<RealVectorValue>(nms::grad(nms::total_energy_density))),
    _grad_pressure(getADMaterialProperty<RealVectorValue>(nms::grad(nms::pressure)))
{
}

ADReal
CNSFluidEnergyAdvectiveFlux::advectedField()
{
  return _specific_total_enthalpy[_qp];
}

ADReal
CNSFluidEnergyAdvectiveFlux::strongResidual()
{
  ADRealVectorValue grad_specific_total_enthalpy = (_grad_rho_et[_qp] + _grad_pressure[_qp]) / _rho[_qp] -
    _specific_total_enthalpy[_qp] / _rho[_qp] * _grad_rho[_qp];
  ADRealVectorValue grad_total_enthalpy_density = _rho[_qp] * grad_specific_total_enthalpy + _specific_total_enthalpy[_qp] * _grad_rho[_qp];

  ADReal value = _eps[_qp] * _rho[_qp] * _specific_total_enthalpy[_qp] * velocityDivergence() +
    _velocity[_qp] * (_eps[_qp] * grad_total_enthalpy_density + _rho[_qp] * _specific_total_enthalpy[_qp] * _grad_eps[_qp]);

  if (_assembly.coordSystem() == Moose::COORD_RZ)
  {
    Real r = _q_point[_qp](_rz_coord);
    value += _eps[_qp] * _specific_total_enthalpy[_qp] * _momentum[_qp](_rz_coord) / r;
  }

  return value;
}
