//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSMomentumTimeDerivative.h"
#include "NS.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSMomentumTimeDerivative);

defineADValidParams(
    CNSMomentumTimeDerivative,
    TimeDerivativeKernel,
    params.addRequiredRangeCheckedParam<unsigned int>(nms::component,
                                                      nms::component + " < 3",
                                                      "equation number (x = 0, y = 1, z = 2)");
    params.addClassDescription("Time derivative $\\epsilon\\frac{\\partial(\\rho_fV_i)}{\\partial t}$ "
                               "in the $i$-th Navier-Stokes and Euler momentum conservation equations."););

CNSMomentumTimeDerivative::CNSMomentumTimeDerivative(const InputParameters & parameters)
  : TimeDerivativeKernel(parameters),
    _component(getParam<unsigned int>(nms::component)),
    _drhou_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::momentum_x))),
    _drhov_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::momentum_y))),
    _drhow_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::momentum_z)))
{
}

ADReal
CNSMomentumTimeDerivative::timeDerivative()
{
  ADRealVectorValue dmomentum(_drhou_dt[_qp], _drhov_dt[_qp], _drhow_dt[_qp]);
  return dmomentum(_component);
}
