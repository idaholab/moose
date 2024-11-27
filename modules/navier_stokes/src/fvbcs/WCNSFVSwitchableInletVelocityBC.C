//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVSwitchableInletVelocityBC.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSFVSwitchableInletVelocityBC);

InputParameters
WCNSFVSwitchableInletVelocityBC::validParams()
{
  InputParameters params = WCNSFVInletVelocityBC::validParams();

  params.addClassDescription("Adds switchable inlet-velocity boundary condition"
                             "for weakly compressible flows.");

  params.addParam<bool>(
      "switch_bc", true, "Switch on (true) / off (false) for boundary condition.");
  params.declareControllable("switch_bc");

  params.addParam<Real>("face_limiter", 1.0, "Face flux limiter.");
  params.declareControllable("face_limiter");

  return params;
}

WCNSFVSwitchableInletVelocityBC::WCNSFVSwitchableInletVelocityBC(const InputParameters & params)
  : WCNSFVInletVelocityBC(params),
    _switch_bc(getParam<bool>("switch_bc")),
    _face_limiter(getParam<Real>("face_limiter"))
{
}

ADReal
WCNSFVSwitchableInletVelocityBC::boundaryValue(const FaceInfo & fi,
                                               const Moose::StateArg & state) const
{
  if (_switch_bc)
    return WCNSFVInletVelocityBC::boundaryValue(fi, state) * _face_limiter;
  else
    // The two-term expansion = false piece is critical as it prevents infinite recursion that would
    // occur with a Green-Gauss gradient calculation which would call back to this "Dirichlet"
    // object
    return _var.getExtrapolatedBoundaryFaceValue(fi,
                                                 /*two_term_expansion=*/false,
                                                 /*correct_skewness=*/false,
                                                 fi.elemPtr(),
                                                 state) *
           _face_limiter;
}
