#include "WettedPerimIC.h"
#include "SubChannelMesh.h"

registerMooseObject("SubChannelApp", WettedPerimIC);

InputParameters
WettedPerimIC::validParams()
{
  return SubChannelBaseIC::validParams();
}

WettedPerimIC::WettedPerimIC(const InputParameters & params) : SubChannelBaseIC(params) {}

Real
WettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  auto pitch = _mesh.getPitch();
  auto rod_diameter = _mesh.getRodDiameter();
  auto gap = _mesh.getGap();
  auto nx = _mesh.getNx();
  auto ny = _mesh.getNy();
  auto rod_circumference = M_PI * rod_diameter;

  // Determine which channel this point is in and if that channel lies at an
  // edge or corner of the assembly.
  auto inds = _mesh.getSubchannelIndexFromPoint(p);
  auto i = inds.first;
  auto j = inds.second;
  bool is_corner = (i == 0 && j == 0) || (i == nx - 1 && j == 0) || (i == 0 && j == ny - 1) ||
                   (i == nx - 1 && j == ny - 1);
  bool is_edge = (i == 0 || j == 0 || i == nx - 1 || j == ny - 1);

  // Compute and return the wetted perimeter.
  if (is_corner)
    return 0.25 * rod_circumference + pitch + 2 * gap;
  else if (is_edge)
    return 0.5 * rod_circumference + pitch;
  else
    return rod_circumference;
}
