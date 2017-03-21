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

// MOOSE includes
#include "IntersectionPointsAlongLine.h"
#include "RayTracing.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<IntersectionPointsAlongLine>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params.addRequiredParam<Point>("start", "The beginning of the line");
  params.addRequiredParam<Point>("end", "The end of the line");
  return params;
}

IntersectionPointsAlongLine::IntersectionPointsAlongLine(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _start(getParam<Point>("start")),
    _end(getParam<Point>("end")),
    _x_intersections(declareVector("x"))
#if LIBMESH_DIM > 1
    ,
    _y_intersections(declareVector("y"))
#if LIBMESH_DIM > 2
    ,
    _z_intersections(declareVector("z"))
#endif
#endif
{
  _intersections.push_back(&_x_intersections);
#if LIBMESH_DIM > 1
  _intersections.push_back(&_y_intersections);
#if LIBMESH_DIM > 2
  _intersections.push_back(&_z_intersections);
#endif
#endif
}

void
IntersectionPointsAlongLine::initialize()
{
  for (unsigned int i = 0; i < _intersections.size(); i++)
    _intersections[i]->clear();
}

void
IntersectionPointsAlongLine::execute()
{
  std::vector<Elem *> intersected_elems;
  std::vector<LineSegment> segments;

  std::unique_ptr<PointLocatorBase> pl = _fe_problem.mesh().getPointLocator();

  // We may not have any elements along the given line; if so then
  // that shouldn't throw a libMesh error.
  pl->enable_out_of_mesh_mode();

  Moose::elementsIntersectedByLine(
      _start, _end, _fe_problem.mesh(), *pl, intersected_elems, segments);

  const unsigned int num_elems = intersected_elems.size();

  // Quick return in case no elements were found
  if (num_elems == 0)
    return;

  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    _intersections[i]->resize(num_elems + 1);

  // Add the beginning point
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    (*_intersections[i])[0] = _start(i);

  // Add the ending point of every segment
  for (unsigned int i = 0; i < num_elems; i++)
  {
    LineSegment & segment = segments[i];

    const Point & end_point = segment.end();

    for (unsigned int j = 0; j < LIBMESH_DIM; j++)
      (*_intersections[j])[i + 1] = end_point(j);
  }
}
