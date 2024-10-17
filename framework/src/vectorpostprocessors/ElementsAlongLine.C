//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementsAlongLine.h"

// MOOSE includes
#include "LineSegment.h"
#include "RayTracing.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", ElementsAlongLine);

InputParameters
ElementsAlongLine::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addRequiredParam<Point>("start", "The beginning of the line");
  params.addRequiredParam<Point>("end", "The end of the line");
  params.addClassDescription("Outputs the IDs of every element intersected by a user-defined line");
  return params;
}

ElementsAlongLine::ElementsAlongLine(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _start(getParam<Point>("start")),
    _end(getParam<Point>("end")),
    _elem_ids(declareVector("elem_ids"))
{
  _fe_problem.mesh().errorIfDistributedMesh("ElementsAlongLine");
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

  std::unique_ptr<libMesh::PointLocatorBase> pl = _fe_problem.mesh().getPointLocator();
  Moose::elementsIntersectedByLine(
      _start, _end, _fe_problem.mesh(), *pl, intersected_elems, segments);

  unsigned int num_elems = intersected_elems.size();

  _elem_ids.resize(num_elems);

  for (unsigned int i = 0; i < num_elems; i++)
    _elem_ids[i] = intersected_elems[i]->id();
}
