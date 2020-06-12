#include "PsbtFlowAreaIC.h"

registerMooseObject("SubChannelApp", PsbtFlowAreaIC);

InputParameters
PsbtFlowAreaIC::validParams()
{
  InputParameters params = PsbtIC::validParams();
  return params;
}

PsbtFlowAreaIC::PsbtFlowAreaIC(const InputParameters & params) : PsbtIC(params) {}

Real
PsbtFlowAreaIC::value(const Point & p)
{
  _mesh = dynamic_cast<SubChannelMesh *>(&_fe_problem.mesh());
  Real pitch = _mesh->pitch_;
  Real rod_diameter = _mesh->rod_diameter_;
  Real gap = _mesh->gap_;
  // Compute the flow area for a standard channel (one that touches 4 pins).
  auto rod_area = 0.25 * M_PI * rod_diameter * rod_diameter;
  auto standard_area = pitch * pitch - rod_area;

  // Determine which channel this point is in and if that channel lies at an
  // edge or corner of the assembly.
  auto inds = index_point(p);
  auto i = inds.first;
  auto j = inds.second;
  bool is_corner = (i == 0 && j == 0) || (i == _mesh->nx_ - 1 && j == 0) ||
                   (i == 0 && j == _mesh->ny_ - 1) || (i == _mesh->nx_ - 1 && j == _mesh->ny_ - 1);
  bool is_edge = (i == 0 || j == 0 || i == _mesh->nx_ - 1 || j == _mesh->ny_ - 1);

  // Compute and return the channel area.
  if (is_corner)
  {
    return standard_area * 0.25 + pitch * gap + gap * gap;
  }
  else if (is_edge)
  {
    return standard_area * 0.5 + pitch * gap;
  }
  else
  {
    return standard_area;
  }
}
