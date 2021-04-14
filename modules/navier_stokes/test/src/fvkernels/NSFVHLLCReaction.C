//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVHLLCReaction.h"
#include "FVUtils.h"
#include "FaceInfo.h"

registerMooseObject("NavierStokesApp", NSFVHLLCReaction);

InputParameters
NSFVHLLCReaction::validParams()
{
  auto params = NSFVHLLCSourceKernel::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<Real>("rate", 1, "The rate of consumption");
  return params;
}

NSFVHLLCReaction::NSFVHLLCReaction(const InputParameters & params)
  : NSFVHLLCSourceKernel(params),
    _component(getParam<MooseEnum>("momentum_component")),
    _rate(getParam<Real>("rate"))

{
}

// ADReal
// NSFVHLLCReaction::computeQpResidual()
// {
//   const auto dxn = (_face_info->neighborCentroid() - _face_info->faceCentroid()) * _normal;
//   auto integral = _rate * dxn * _u_neighbor[_qp];
//   const auto dxe = (_face_info->faceCentroid() - _face_info->elemCentroid()) * _normal;
//   integral += _rate * dxe * _u_elem[_qp];

//   return integral;

//   // ADReal u;
//   // Moose::FV::interpolate(
//   //     Moose::FV::InterpMethod::Average, u, _u_elem[_qp], _u_neighbor[_qp], *_face_info, true);

//   // return _rate * dx * u;
// }

ADReal
NSFVHLLCReaction::sourceElem()
{
  return _rate * _u_elem[_qp];
}

ADReal
NSFVHLLCReaction::sourceNeighbor()
{
  return _rate * _u_neighbor[_qp];
}
