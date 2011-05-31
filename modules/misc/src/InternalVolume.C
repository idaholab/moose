#include "InternalVolume.h"

template <>
InputParameters validParams<InternalVolume>()
{
  InputParameters params = validParams<SideIntegral>();
  params.addParam<unsigned int>("component", 0, "The component to use in the integration");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

InternalVolume::InternalVolume(const std::string & name,
                               InputParameters parameters)
  : SideIntegral( name, parameters ),
    _component( getParam<unsigned int>("component") )
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
  return -_q_point[_qp](_component)*_normals[_qp](_component);
}
