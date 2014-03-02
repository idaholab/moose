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

#include "NearestPointLayeredAverage.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<NearestPointLayeredAverage>()
{
  InputParameters params = validParams<ElementIntegralVariableUserObject>();

  params.addRequiredParam<std::vector<Real> >("points", "Layered averages will be computed in space closest to these points.");

  params += validParams<LayeredAverage>();

  return params;
}

NearestPointLayeredAverage::NearestPointLayeredAverage(const std::string & name, InputParameters parameters) :
    ElementIntegralVariableUserObject(name, parameters)
{
  const std::vector<Real> & points_vec = getParam<std::vector<Real> >("points");

  {
    unsigned int num_vec_entries = points_vec.size();

    mooseAssert(num_vec_entries % LIBMESH_DIM == 0, "Wrong number of entries in 'points'");

    _points.reserve(num_vec_entries / LIBMESH_DIM);

    // Read the points out of the vector
    for(unsigned int i=0; i<num_vec_entries; i+=3)
      _points.push_back(Point(points_vec[i], points_vec[i+1], points_vec[i+2]));
  }

  _layered_averages.reserve(_points.size());

  // Build each of the LayeredAverage objects:
  for(unsigned int i=0; i<_points.size(); i++)
    _layered_averages.push_back(new LayeredAverage(name, parameters));
}

NearestPointLayeredAverage::~NearestPointLayeredAverage()
{
  for(unsigned int i=0; i<_layered_averages.size(); i++)
    delete _layered_averages[i];
}

void
NearestPointLayeredAverage::initialize()
{
  for(unsigned int i=0; i<_layered_averages.size(); i++)
    _layered_averages[i]->initialize();
}

void
NearestPointLayeredAverage::execute()
{
  nearestLayeredAverage(_current_elem->centroid())->execute();
}

void
NearestPointLayeredAverage::finalize()
{
  for(unsigned int i=0; i<_layered_averages.size(); i++)
    _layered_averages[i]->finalize();
}

void
NearestPointLayeredAverage::threadJoin(const UserObject & y)
{
  const NearestPointLayeredAverage & npla = static_cast<const NearestPointLayeredAverage &>(y);

  for(unsigned int i=0; i<_layered_averages.size(); i++)
    _layered_averages[i]->threadJoin(*npla._layered_averages[i]);
}

LayeredAverage *
NearestPointLayeredAverage::nearestLayeredAverage(const Point & p) const
{
  unsigned int closest = 0;
  Real closest_distance = std::numeric_limits<Real>::max();

  for(unsigned int i=0; i<_points.size(); i++)
  {
    const Point & current_point = _points[i];

    Real current_distance = (p - current_point).size();

    if (current_distance < closest_distance)
    {
      closest_distance = current_distance;
      closest = i;
    }
  }

  return _layered_averages[closest];
}
