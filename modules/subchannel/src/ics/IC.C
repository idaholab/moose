#include "IC.h"
#include "SubChannelMesh.h"

InputParameters
IC::validParams(){ return InitialCondition::validParams();}

IC::IC(const InputParameters & params) : InitialCondition(params) ,
_mesh(dynamic_cast<SubChannelMesh &> (_fe_problem.mesh())) {}

std::pair<unsigned int, unsigned int>
IC::index_point(const Point & p) const
{
  auto pitch = _mesh._pitch;
  unsigned int i = (p(0) + 0.5 * pitch) / pitch;
  unsigned int j = (p(1) + 0.5 * pitch) / pitch;
  return {i, j};
}
