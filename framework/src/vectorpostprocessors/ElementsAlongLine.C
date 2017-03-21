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
#include "ElementsAlongLine.h"
#include "RayTracing.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ElementsAlongLine>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<Point>("start", "The beginning of the line");
  params.addRequiredParam<Point>("end", "The end of the line");
  return params;
}

ElementsAlongLine::ElementsAlongLine(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _start(getParam<Point>("start")),
    _end(getParam<Point>("end")),
    _elem_ids(declareVector("elem_ids"))
{
}

void
ElementsAlongLine::initialize()
{
  _elem_ids.clear();
}

void
ElementsAlongLine::execute()
{
  std::vector<Elem *> intersected_elems;
  std::vector<LineSegment> segments;

  std::unique_ptr<PointLocatorBase> pl = _fe_problem.mesh().getPointLocator();
  Moose::elementsIntersectedByLine(
      _start, _end, _fe_problem.mesh(), *pl, intersected_elems, segments);

  unsigned int num_elems = intersected_elems.size();

  _elem_ids.resize(num_elems);

  for (unsigned int i = 0; i < num_elems; i++)
    _elem_ids[i] = intersected_elems[i]->id();
}
