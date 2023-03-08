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

InputParameters
MeshMetaDataDependenceAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredCoupledVar("variable", "The name of the variable to sample");
  params.addRequiredParam<MeshGeneratorName>(
      "mesh_generator",
      "The name of the generator to use as the prefix for mesh meta data properties.");

  params.addParam<bool>("test_request_invalid_property",
                        false,
                        "Set to true to have this object request a property that isn't available");

  return params;
}

MeshMetaDataDependenceAction::MeshMetaDataDependenceAction(const InputParameters & params)
  : Action(params), _generator_prefix(getParam<MeshGeneratorName>("mesh_generator"))
{
}

void
MeshMetaDataDependenceAction::act()
{
  if (getParam<bool>("test_request_invalid_property"))
    static_cast<void>(getMeshProperty<Real>("nonexistent_property", _generator_prefix));

  const auto num_elements_x_prop =
      getMeshProperty<unsigned int>("num_elements_x", _generator_prefix);
  const auto xmin_prop = getMeshProperty<Real>("xmin", _generator_prefix);
  const auto xmax_prop = getMeshProperty<Real>("xmax", _generator_prefix);

  // First query the data store to get a mesh attributes
  Point start_point = Point(xmin_prop, 0, 0);
  Point end_point = Point(xmax_prop, 0, 0);

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
  vpp_params.set<unsigned int>("num_points") = num_elements_x_prop + 1;

  _problem->addVectorPostprocessor(type, "line_sampler_between_elems", vpp_params);
}
