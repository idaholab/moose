//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVHLLCReactionBC.h"
#include "FVUtils.h"
#include "FaceInfo.h"

registerMooseObject("NavierStokesApp", NSFVHLLCReactionBC);

InputParameters
NSFVHLLCReactionBC::validParams()
{
  auto params = NSFVHLLCSourceBC::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<Real>("rate", 1, "The rate of consumption");
  return params;
}

NSFVHLLCReactionBC::NSFVHLLCReactionBC(const InputParameters & params)
  : NSFVHLLCSourceBC(params),
    _component(getParam<MooseEnum>("momentum_component")),
    _rate(getParam<Real>("rate"))
{
}

// ADReal
// NSFVHLLCReactionBC::computeQpResidual()
// {
//   const auto dx = (_face_info->faceCentroid() - _face_info->elemCentroid()) * _normal;

//   const ADReal & u = (_face_info->faceType(_var.name()) == FaceInfo::VarFaceNeighbors::ELEM)
//                          ? _u[_qp]
//                          : _u_neighbor[_qp];

//   return _rate * dx * u;
// }

ADReal
NSFVHLLCReactionBC::sourceElem()
{
  return _rate * _u[_qp];
}
