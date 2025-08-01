//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  InputParameters params = INSFVOutletPressureBCTempl<INSFVFlowBC>::validParams();

  params.addClassDescription("Adds switchable pressure-outlet boundary condition");

  params.addParam<bool>(
      "switch_bc", true, "Switch on (true) / off (false) for boundary condition.");
  params.declareControllable("switch_bc");

  params.addParam<Real>("face_limiter", 1.0, "Face flux limiter.");
  params.declareControllable("face_limiter");

  return params;
}

INSFVSwitchableOutletPressureBC::INSFVSwitchableOutletPressureBC(const InputParameters & params)
  : INSFVOutletPressureBCTempl<INSFVFlowBC>(params),
    _switch_bc(getParam<bool>("switch_bc")),
    _face_limiter(getParam<Real>("face_limiter"))
{
}

ADReal
INSFVSwitchableOutletPressureBC::boundaryValue(const FaceInfo & fi,
                                               const Moose::StateArg & state) const
{
  if (_switch_bc)
    return INSFVOutletPressureBCTempl<INSFVFlowBC>::boundaryValue(fi, state) * _face_limiter;
  else
  {
    // if on an internal face (internal to the mesh, but an external boundary of the flow area),
    // we have to make sure to select the element on which the pressure is defined
    const auto elem_ptr = (fi.faceType(_var_sys_numbers_pair) == FaceInfo::VarFaceNeighbors::ELEM)
                              ? fi.elemPtr()
                              : fi.neighborPtr();
    // The two-term expansion = false piece is critical as it prevents infinite recursion that would
    // occur with a Green-Gauss gradient calculation which would call back to this "Dirichlet"
    // object
    return _var.getExtrapolatedBoundaryFaceValue(fi,
                                                 /*two_term_expansion=*/false,
                                                 /*correct_skewness=*/false,
                                                 elem_ptr,
                                                 state) *
           _face_limiter;
  }
}
