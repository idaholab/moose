#include "SubChannelBaseIC.h"
#include "SubChannelMesh.h"

InputParameters
SubChannelBaseIC::validParams(){ return InitialCondition::validParams();}

SubChannelBaseIC::SubChannelBaseIC(const InputParameters & params) : InitialCondition(params) ,
_mesh(dynamic_cast<SubChannelMesh &> (_fe_problem.mesh())) {}

std::pair<unsigned int, unsigned int>
SubChannelBaseIC::index_point(const Point & p) const
{
  auto pitch = _mesh._pitch;
  unsigned int i = (p(0) + 0.5 * pitch) / pitch;
  unsigned int j = (p(1) + 0.5 * pitch) / pitch;
  return {i, j};
}
