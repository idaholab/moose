//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSMomentumPressureGradient.h"
#include "NS.h"
#include "Assembly.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", CNSMomentumPressureGradient);

defineADValidParams(
    CNSMomentumPressureGradient,
    CNSKernel,
    params.addRequiredCoupledVar(nms::porosity, "porosity");
    params.addParam<bool>("divergence", true,
      "whether to apply the divergence theorem to this kernel");
    params.addRequiredRangeCheckedParam<unsigned int>(nms::component,
                                                      nms::component + " < 3",
                                                      "equation number (x = 0, y = 1, z = 2)");
    params.addClassDescription("Pressure gradient $\\epsilon\\frac{\\partial P}{\\partial x_i}$ "
                               "in the $i$-th momentum conservation equation."););

CNSMomentumPressureGradient::CNSMomentumPressureGradient(const InputParameters & parameters)
  : CNSKernel(parameters),
    _divergence(getParam<bool>("divergence")),
    _component(getParam<unsigned int>(nms::component)),
    _eps(coupledValue(nms::porosity)),
    _grad_eps(coupledGradient(nms::porosity)),
    _pressure(getADMaterialProperty<Real>(nms::pressure)),
    _grad_pressure(getADMaterialProperty<RealVectorValue>(nms::grad(nms::pressure)))
{
}

ADReal
CNSMomentumPressureGradient::weakResidual()
{
  if (_divergence) {
    ADReal value = -_eps[_qp] * _pressure[_qp] * _grad_test[_i][_qp](_component) -
      _pressure[_qp] * _grad_eps[_qp](_component) * _test[_i][_qp];

    if ((_assembly.coordSystem() == Moose::COORD_RZ) && (_component == _rz_coord)) {
      ADReal r = _q_point[_qp](_rz_coord);
      value += -_eps[_qp] * _pressure[_qp] / r * _test[_i][_qp];
    }

   return value;
  }
  else
    return _eps[_qp] * _grad_pressure[_qp](_component) * _test[_i][_qp];
}

ADReal
CNSMomentumPressureGradient::strongResidual()
{
  return _eps[_qp] * _grad_pressure[_qp](_component);
}
