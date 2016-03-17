/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "InternalVolume.h"

template <>
InputParameters validParams<InternalVolume>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  params.addParam<unsigned int>("component", 1, "The component to use in the integration");
  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the internal volume calculation");
  params.addParam<Real>("addition", 0, "An additional volume to be included in the internal volume calculation");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

InternalVolume::InternalVolume(const InputParameters & parameters) :
    SideIntegralPostprocessor(parameters),
    _component( getParam<unsigned int>("component") ),
    _scale( getParam<Real>("scale_factor") ),
    _addition( getParam<Real>("addition") )
{
}

//    /              /
//   |              |
//   |  div(F) dV = | F dot n dS
//   |              |
//  / V            / dS
//
// with
//   F = a field
//   n = the normal at the surface
//   V = the volume of the domain
//   S = the surface of the domain
//
// If we choose F as [x 0 0]^T, then
//   div(F) = 1.
// So,
//
//    /       /
//   |       |
//   |  dV = | x * n[0] dS
//   |       |
//  / V     / dS
//
// That is, the volume of the domain is the integral over the surface of the domain
// of the x position of the surface times the x-component of the normal of the
// surface.
//
Real
InternalVolume::computeQpIntegral()
{
  const Real scale =_coord_sys == Moose::COORD_RSPHERICAL ? 1.0/3.0 : 1.0;
  return -scale * _q_point[_qp](_component) * _normals[_qp](_component);
}

Real
InternalVolume::getValue()
{
  return _scale * SideIntegralPostprocessor::getValue() + _addition;
}
