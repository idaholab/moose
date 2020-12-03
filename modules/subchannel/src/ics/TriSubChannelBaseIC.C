#include "TriSubChannelBaseIC.h"
#include "TriSubChannelMesh.h"

InputParameters
TriSubChannelBaseIC::validParams()
{
  return InitialCondition::validParams();
}

TriSubChannelBaseIC::TriSubChannelBaseIC(const InputParameters & params)
  : InitialCondition(params), _mesh(dynamic_cast<TriSubChannelMesh &>(_fe_problem.mesh()))
{
}

unsigned int
TriSubChannelBaseIC::index_point(const Point & p) const
{
  Real distance0 = 1.0e+8;
  Real distance1;
  unsigned int j = 0;

  for (unsigned int i = 0; i < _mesh._n_channels; i++)
  {
    distance1 = std::sqrt(std::pow((p(0) - _mesh._subchannel_position[i][0]), 2.0) +
                          std::pow((p(1) - _mesh._subchannel_position[i][1]), 2.0));

    if (distance1 < distance0)
    {
      j = i;
      distance0 = distance1;
    } // if
  }   // for
  return j;
}
