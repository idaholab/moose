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

  params.addParam<bool>("switch", true, "Switch on (true) / off (false) for boundary condition.");
  params.declareControllable("switch");

  params.addParam<Real>("face_limiter", 1.0, "Interpolation face flux limiter.");
  params.declareControllable("face_limiter");

  return params;
}

WCNSFVSwitchableInletVelocityBC::WCNSFVSwitchableInletVelocityBC(const InputParameters & params)
  : WCNSFVInletVelocityBC(params),
    _switch_bc(getParam<bool>("switch")),
    _face_limiter(getParam<Real>("face_limiter"))
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    paramError("variable",
               "The variable argument to WCNSFVSwitchableInletVelocityBC must be of type "
               "INSFVVelocityVariable");

  // Density is often set as global parameters so it is not checked
  if (_velocity_pp && (_mdot_pp || _area_pp))
    mooseWarning("If setting the velocity directly, no need for inlet mass flow rate or area");

  // Need enough information if trying to use a mass flow rate postprocessor
  if (!_velocity_pp && (!_mdot_pp || !_area_pp || !_rho))
    mooseError("Mass flow rate, area and density should be provided if velocity is not");
}

ADReal
WCNSFVSwitchableInletVelocityBC::boundaryValue(const FaceInfo & fi) const
{
  if (_switch_bc)
    return WCNSFVInletVelocityBC::boundaryValue(fi) * _face_limiter;
  else
    return _var.getExtrapolatedBoundaryFaceValue(fi, false, false, fi.elemPtr(), determineState()) *
           _face_limiter;
}
