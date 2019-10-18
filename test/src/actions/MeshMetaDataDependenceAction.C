//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshMetaDataDependenceAction.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"
#include "MooseApp.h"
#include "FEProblem.h"

#include "libmesh/vector_value.h"

registerMooseAction("MooseTestApp", MeshMetaDataDependenceAction, "add_vector_postprocessor");

template <>
InputParameters
validParams<MeshMetaDataDependenceAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredCoupledVar("variable", "The name of the variable to sample");

  return params;
}

MeshMetaDataDependenceAction::MeshMetaDataDependenceAction(const InputParameters & params)
  : Action(params)
{
}

void
MeshMetaDataDependenceAction::act()
{
  // First query the data store to get a mesh attributes
  Point start_point = Point(getMeshProperty<Real>("xmin"), 0, 0);
  Point end_point = Point(getMeshProperty<Real>("xmax"), 0, 0);
  auto num_elem_x = getMeshProperty<unsigned int>("num_elements_x");

  std::cout << "start_point: " << start_point << '\n';
  std::cout << "end_point: " << end_point << '\n';
  std::cout << "num_elem_x: " << num_elem_x << '\n';

  // Using that information, let's add a VectorPostprocessor based upon the attribute.
  // Note: This is not the way a user should do this, we have VectorPostprocessors that
  // automatically along a line of elements.
  const std::string type = "LineValueSampler";
  auto vpp_params = _factory.getValidParams(type);

  vpp_params.applyParameters(_pars);

  vpp_params.set<Point>("start_point") = start_point;
  vpp_params.set<Point>("end_point") = end_point;
  vpp_params.set<MooseEnum>("sort_by") = "x";

  // line up samples along nodes
  vpp_params.set<unsigned int>("num_points") = num_elem_x + 1;

  _problem->addVectorPostprocessor(type, "line_sampler_between_elems", vpp_params);
}
