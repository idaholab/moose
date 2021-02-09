//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSMomentumAdvectiveFlux.h"
#include "NS.h"
#include "Assembly.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSMomentumAdvectiveFlux);

defineADValidParams(
    CNSMomentumAdvectiveFlux,
    AdvectiveFluxKernel,
    params.addRequiredRangeCheckedParam<unsigned int>(nms::component,
                                                      nms::component + " < 3",
                                                      "Equation number (x = 0, y = 1, z = 2)");
   params.addClassDescription("Advective flux $\\nabla\\cdot(\\epsilon\\rho_fV_i\\vec{V})$ "
                              "in the $i$-th Navier-Stokes and Euler momentum conservation equations."););

CNSMomentumAdvectiveFlux::CNSMomentumAdvectiveFlux(const InputParameters & parameters)
  : AdvectiveFluxKernel(parameters),
    _component(getParam<unsigned int>(nms::component)),
    _momentum(getADMaterialProperty<RealVectorValue>(nms::momentum))
{
}

ADReal
CNSMomentumAdvectiveFlux::advectedField()
{
  return _velocity[_qp](_component);
}

ADReal
CNSMomentumAdvectiveFlux::strongResidual()
{
  ADRealVectorValue grad_mom_vec =
    _component == 0 ? _grad_rho_u[_qp] : (_component == 1 ? _grad_rho_v[_qp] : _grad_rho_w[_qp]);

  ADReal value = _eps[_qp] * _momentum[_qp](_component) * velocityDivergence() +
    _velocity[_qp] * (_eps[_qp] * grad_mom_vec + _momentum[_qp](_component) * _grad_eps[_qp]);

  if (_assembly.coordSystem() == Moose::COORD_RZ)
  {
    ADReal r = _q_point[_qp](_rz_coord);
    value += _eps[_qp] * _momentum[_qp](_component) * _velocity[_qp](_rz_coord) / r;
  }

  return value;
}
