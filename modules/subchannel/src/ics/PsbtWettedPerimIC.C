#include "PsbtWettedPerimIC.h"

registerMooseObject("SubChannelApp", PsbtWettedPerimIC);

InputParameters
PsbtWettedPerimIC::validParams()
{
  return PsbtIC::validParams();
}

PsbtWettedPerimIC::PsbtWettedPerimIC(const InputParameters & params) : PsbtIC(params) {}

Real
PsbtWettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  _mesh = dynamic_cast<SubChannelMesh *>(&_fe_problem.mesh());
  auto pitch = _mesh->_pitch;
  auto rod_diameter = _mesh->_rod_diameter;
  auto gap = _mesh->_gap;
  auto rod_circumference = M_PI * rod_diameter;

  // Determine which channel this point is in and if that channel lies at an
  // edge or corner of the assembly.
  auto inds = index_point(p);
  auto i = inds.first;
  auto j = inds.second;
  bool is_corner = (i == 0 && j == 0) || (i == _mesh->_nx - 1 && j == 0) ||
                   (i == 0 && j == _mesh->_ny - 1) || (i == _mesh->_nx - 1 && j == _mesh->_ny - 1);
  bool is_edge = (i == 0 || j == 0 || i == _mesh->_nx - 1 || j == _mesh->_ny - 1);

  // Compute and return the wetted perimeter.
  if (is_corner)
    return 0.25 * rod_circumference + pitch + 2 * gap;
  else if (is_edge)
    return 0.5 * rod_circumference + pitch;
  else
    return rod_circumference;
}
