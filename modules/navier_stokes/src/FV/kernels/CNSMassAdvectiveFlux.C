//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSMassAdvectiveFlux.h"
#include "NS.h"
#include "Assembly.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSMassAdvectiveFlux);

defineADValidParams(CNSMassAdvectiveFlux,
                    AdvectiveFluxKernel,
                    params.addClassDescription("Advective flux $\\nabla\\cdot(\\epsilon\\rho_f\\vec{V})$ "
                                               "in the mass conservation equation."););

CNSMassAdvectiveFlux::CNSMassAdvectiveFlux(const InputParameters & parameters)
  : AdvectiveFluxKernel(parameters)
{
}

ADReal
CNSMassAdvectiveFlux::advectedField()
{
  return 1.0;
}

ADReal
CNSMassAdvectiveFlux::strongResidual()
{
  ADReal value = _eps[_qp] * _rho[_qp] * velocityDivergence() +
    _velocity[_qp] * (_eps[_qp] * _grad_rho[_qp] + _rho[_qp] * _grad_eps[_qp]);

  if (_assembly.coordSystem() == Moose::COORD_RZ)
  {
    ADReal r = _q_point[_qp](_rz_coord);
    value += _eps[_qp] * _rho[_qp] * _velocity[_qp](_rz_coord) / r;
  }

  return value;
}
