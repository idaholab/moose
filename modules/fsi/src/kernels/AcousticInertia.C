//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AcousticInertia.h"
#include "SubProblem.h"
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"

registerMooseObject("FsiApp", AcousticInertia);

InputParameters
AcousticInertia::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addClassDescription("Calculates the residual for the inertial force "
                             "which is the double time derivative of pressure.");
  params.addParam<MaterialPropertyName>(
      "inv_co_sq", "inv_co_sq", "Inverse of sqaure of the fluid's speed of sound.");
  return params;
}

AcousticInertia::AcousticInertia(const InputParameters & parameters)
  : TimeKernel(parameters),
    _inv_co_sq(getMaterialProperty<Real>("inv_co_sq")),
    _u_dot_old(_var.uDotOld()),
    _du_dot_du(_var.duDotDu()),
    _du_dotdot_du(_var.duDotDotDu()),
    _u_dot_factor(_var.vectorTagValue(_sys.getTimeIntegrator(_var.number()).uDotFactorTag())),
    _u_dotdot_factor(_var.vectorTagValue(_sys.getTimeIntegrator(_var.number()).uDotDotFactorTag()))
{
  addFEVariableCoupleableVectorTag(_sys.getTimeIntegrator(_var.number()).uDotFactorTag());
  addFEVariableCoupleableVectorTag(_sys.getTimeIntegrator(_var.number()).uDotDotFactorTag());
}

Real
AcousticInertia::computeQpResidual()
{
  if (_dt == 0)
    return 0;
  else
    return _test[_i][_qp] * _inv_co_sq[_qp] *
           (_u_dotdot_factor[_qp] + _u_dot_factor[_qp] - _u_dot_old[_qp]);
}

Real
AcousticInertia::computeQpJacobian()
{
  if (_dt == 0)
    return 0;
  else
    return _test[_i][_qp] * _inv_co_sq[_qp] * _du_dotdot_du[_qp] * _phi[_j][_qp] +
           _test[_i][_qp] * _inv_co_sq[_qp] * _du_dot_du[_qp] * _phi[_j][_qp];
}
