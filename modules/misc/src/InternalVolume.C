/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "InternalVolume.h"

template <>
InputParameters validParams<InternalVolume>()
{
  InputParameters params = validParams<SideIntegral>();
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

InternalVolume::InternalVolume(const std::string & name,
                               InputParameters parameters)
  : SideIntegral( name, parameters )
{}

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
  return -_q_point[_qp](0)*_normals[_qp](0);
}
