//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVSwitchableOutletPressureBC.h"
#include "INSFVPressureVariable.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSFVSwitchableOutletPressureBC);

InputParameters
INSFVSwitchableOutletPressureBC::validParams()
{
  InputParameters params = INSFVOutletPressureBC::validParams();

  params.addClassDescription("Adds switchable pressure-outlet boundary condition");

  params.addParam<bool>("switch", true, "Switch on (true) / off (false) for boundary condition.");
  params.declareControllable("switch");

  params.addParam<Real>("face_limiter", 1.0, "Interpolation face flux limiter.");
  params.declareControllable("face_limiter");

  return params;
}

INSFVSwitchableOutletPressureBC::INSFVSwitchableOutletPressureBC(const InputParameters & params)
  : INSFVOutletPressureBC(params),
    _switch_bc(getParam<bool>("switch")),
    _face_limiter(getParam<Real>("face_limiter"))
{
  if (!dynamic_cast<INSFVPressureVariable *>(&_var))
    paramError("variable",
               "The variable argument to INSFVSwitchableOutletPressureBC must be of type "
               "INSFVPressureVariable");

  // Check parameters
  if ((_functor && (_pp_value || _function)) || (_function && _pp_value) ||
      (!_functor && !_pp_value && !_function))
    mooseError("One and only one of function/functor/postprocessor may be specified for the outlet "
               "pressure");
}

ADReal
INSFVSwitchableOutletPressureBC::boundaryValue(const FaceInfo & fi) const
{
  if (_switch_bc)
    return INSFVOutletPressureBC::boundaryValue(fi) * _face_limiter;
  else
    return _var.getExtrapolatedBoundaryFaceValue(fi, false, false, fi.elemPtr(), determineState()) *
           _face_limiter;
}
