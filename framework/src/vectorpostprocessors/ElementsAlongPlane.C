//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ElementsAlongPlane.h"
#include "ElementsIntersectedByPlane.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", ElementsAlongPlane);

InputParameters
ElementsAlongPlane::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addRequiredParam<Point>("point", "Point in the plane");
  params.addRequiredParam<Point>("normal", "Normal vector to the plane");
  params.addClassDescription(
      "Outputs the IDs of every element intersected by a user-defined plane");
  return params;
}

ElementsAlongPlane::ElementsAlongPlane(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _p0(getParam<Point>("point")),
    _normal(getParam<Point>("normal")),
    _elem_ids(declareVector("elem_ids"))
{
  _fe_problem.mesh().errorIfDistributedMesh("ElementsAlongPlane");
}

void
ElementsAlongPlane::initialize()
{
  _elem_ids.clear();
}

void
ElementsAlongPlane::execute()
{
  std::vector<const Elem *> intersected_elems;

  Moose::elementsIntersectedByPlane(_p0, _normal, _fe_problem.mesh(), intersected_elems);

  unsigned int num_elems = intersected_elems.size();

  _elem_ids.resize(num_elems);

  for (unsigned int i = 0; i < num_elems; i++)
    _elem_ids[i] = intersected_elems[i]->id();
}
