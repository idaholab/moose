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
#include "ElementsAlongPlane.h"
#include "ElementsIntersectedByPlane.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ElementsAlongPlane>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<Point>("point", "Point in the plane");
  params.addRequiredParam<Point>("normal", "Normal vector to the plane");
  return params;
}

ElementsAlongPlane::ElementsAlongPlane(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _p0(getParam<Point>("point")),
    _normal(getParam<Point>("normal")),
    _elem_ids(declareVector("elem_ids"))
{
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
