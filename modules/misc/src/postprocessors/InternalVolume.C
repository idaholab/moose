//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalVolume.h"

#include "MooseMesh.h"
#include "Function.h"

registerMooseObject("MiscApp", InternalVolume);

InputParameters
InternalVolume::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription("Computes the volume of an enclosed area by "
                             "performing an integral over a user-supplied boundary.");
  params.addRangeCheckedParam<unsigned int>(
      "component", 0, "component<3", "The component to use in the integration");
  params.addParam<Real>(
      "scale_factor", 1, "A scale factor to be applied to the internal volume calculation");
  params.addParam<FunctionName>("addition",
                                0,
                                "An additional volume to be included in the "
                                "internal volume calculation. A time-dependent "
                                "function is expected.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

InternalVolume::InternalVolume(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _component(getParam<unsigned int>("component")),
    _scale(getParam<Real>("scale_factor")),
    _addition(getFunction("addition"))
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

void
InternalVolume::initialSetup()
{
  SideIntegralPostprocessor::initialSetup();

  const std::set<SubdomainID> & subdomains = _mesh.meshSubdomains();
  std::set<SubdomainID>::const_iterator iter = subdomains.begin();
  const std::set<SubdomainID>::const_iterator iter_end = subdomains.end();
  for (; iter != iter_end; ++iter)
  {
    const std::set<BoundaryID> & boundaries = _mesh.getSubdomainBoundaryIds(*iter);
    std::set<BoundaryID>::const_iterator bnd = boundaries.begin();
    const std::set<BoundaryID>::const_iterator bnd_end = boundaries.end();
    for (; bnd != bnd_end; ++bnd)
    {
      const std::set<BoundaryID> & b = boundaryIDs();
      std::set<BoundaryID>::const_iterator bit = b.begin();
      const std::set<BoundaryID>::const_iterator bit_end = b.end();
      for (; bit != bit_end; ++bit)
        if (*bit == *bnd)
        {
          Moose::CoordinateSystemType coord_sys = _fe_problem.getCoordSystem(*iter);
          if (_component != 0 && coord_sys == Moose::COORD_RSPHERICAL)
            mooseError("With spherical coordinates, the component must be 0 in InternalVolume.");

          if (_component > 1 && coord_sys == Moose::COORD_RZ)
            mooseError(
                "With cylindrical coordinates, the component must be 0 or 1 in InternalVolume.");
        }
    }
  }
}

Real
InternalVolume::computeQpIntegral()
{
  // Default scale factor is 1
  Real scale = 1.0;
  if (_coord_sys == Moose::COORD_RSPHERICAL)
  {
    // MOOSE will multiply by 4*pi*r*r
    scale = 1.0 / 3.0;
  }
  else if (_coord_sys == Moose::COORD_RZ && _component == 0)
  {
    // MOOSE will multiply by 2*pi*r
    // Will integrate over z giving 0.5*2*pi*r*r*height
    scale = 0.5;
  }
  else if (_coord_sys == Moose::COORD_RZ && _component == 1)
  {
    // MOOSE will multiply by 2*pi*r
    // Will integrate over r:
    // integral(2*pi*r*height) over r:
    // pi*r*r*height
    scale = 1.0;
  }
  return -scale * _q_point[_qp](_component) * _normals[_qp](_component);
}

Real
InternalVolume::getValue()
{
  return _scale * SideIntegralPostprocessor::getValue() + _addition.value(_t);
}
