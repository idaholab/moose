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
  params.addRequiredParam<Real>("penalty", "The penalty");
  params.addRequiredParam<NonlinearVariableName>("u1", "The x-velocity on the '1' side");
  params.addRequiredParam<NonlinearVariableName>("u2", "The x-velocity on the '2' side");
  return params;
}

PINSFVPenaltyMassContinuity::PINSFVPenaltyMassContinuity(const InputParameters & params)
  : FVInterfaceKernel(params),
    _penalty(getParam<Real>("penalty")),
    _u1(_sys.getFVVariable<Real>(_tid, getParam<NonlinearVariableName>("u1"))),
    _u2(_sys.getFVVariable<Real>(_tid,
                                 isParamValid("u2") ? getParam<NonlinearVariableName>("u2")
                                                    : getParam<NonlinearVariableName>("u1")))
{
}

ADReal
PINSFVPenaltyMassContinuity::computeQpResidual()
{
  const auto u1 = _u1.getBoundaryFaceValue(*_face_info);
  const auto u2 = _u2.getBoundaryFaceValue(*_face_info);
  return _penalty * (u1 - u2);
}
