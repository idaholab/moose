//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVPenaltyMassContinuity.h"
#include "NS.h"
#include "SystemBase.h"
#include "Assembly.h"

registerMooseObject("NavierStokesApp", PINSFVPenaltyMassContinuity);

InputParameters
PINSFVPenaltyMassContinuity::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.set<unsigned short>("ghost_layers") = 2;
  params.addRequiredParam<NonlinearVariableName>("u1", "The x-velocity on the '1' side");
  params.addRequiredParam<NonlinearVariableName>("u2", "The x-velocity on the '2' side");
  params.addRequiredParam<Real>(NS::density, "The density");
  return params;
}

PINSFVPenaltyMassContinuity::PINSFVPenaltyMassContinuity(const InputParameters & params)
  : FVInterfaceKernel(params),
    _u1(_sys.getFVVariable<Real>(_tid, getParam<NonlinearVariableName>("u1"))),
    _u2(_sys.getFVVariable<Real>(_tid,
                                 isParamValid("u2") ? getParam<NonlinearVariableName>("u2")
                                                    : getParam<NonlinearVariableName>("u1"))),
    _rho(getParam<Real>(NS::density))
{
}

ADReal
PINSFVPenaltyMassContinuity::computeQpResidual()
{
  const auto u1 = _u1.getBoundaryFaceValue(*_face_info);
  const auto u2 = _u2.getBoundaryFaceValue(*_face_info);
  ADReal u_face;
  Moose::FV::interpolate(
      Moose::FV::InterpMethod::Average, u_face, u1, u2, *_face_info, _elem_is_one);
  return _rho * u_face * _face_info->normal()(0);
}
