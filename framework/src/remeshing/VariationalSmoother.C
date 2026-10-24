//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariationalSmoother.h"

#include "MooseMesh.h"

#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/mesh_smoother_vsmoother.h"
#include "libmesh/node.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/utility.h"

using namespace libMesh;

registerMooseObject("MooseApp", VariationalSmoother);

InputParameters
VariationalSmoother::validParams()
{
  InputParameters params = LaplaceSmoother::validParams();

  params.addClassDescription(
      "Moves the mesh between remesh events with a harmonic interpolation of the prescribed "
      "boundary motion as a predictor and libMesh's variational mesh smoother as a corrector, "
      "which relocates the nodes to optimize the distortion-dilation quality metric of the "
      "elements.");

  params.addRangeCheckedParam<Real>(
      "dilation_weight",
      0.5,
      "dilation_weight >= 0 & dilation_weight <= 1",
      "The weight of the dilation (volume) metric in the corrector's objective. The distortion "
      "(shape) metric is weighted with one minus this.");

  return params;
}

VariationalSmoother::VariationalSmoother(const InputParameters & parameters)
  : LaplaceSmoother(parameters), _dilation_weight(getParam<Real>("dilation_weight"))
{
}

void
VariationalSmoother::updatePseudoDisplacement(const Real dt)
{
  // The harmonic predictor writes the total pseudo-displacement d over every node this rank holds
  LaplaceSmoother::updatePseudoDisplacement(dt);

  auto & mesh = _mesh.getMesh();
  const auto & x0 = referenceCoordinates();
  auto & d = pseudoDisplacement();

  // The engine only applies x = X0 + d after this returns, so the predicted configuration is
  // placed here for the corrector to see it
  placeNodesAtPseudoDisplacement();

  // The corrector constrains every boundary node from the current geometry itself: a node on a
  // flat stretch of boundary slides along it and a corner is pinned where it sits, so the
  // predicted boundary keeps its shape while the nodes rearrange for element quality
  VariationalMeshSmoother smoother(cast_ref<UnstructuredMesh &>(mesh), _dilation_weight);
  try
  {
    smoother.smooth();
  }
  catch (const std::exception & e)
  {
    // The positions the diverged solve left in the mesh would be written into d below, so it can
    // never be allowed to pass silently
    mooseError("The variational smoothing solve failed: ", e.what());
  }

  // smooth() synchronized the new positions over every node this rank holds, so the total
  // pseudo-displacement is read straight back off the mesh
  for (const auto & node : mesh.node_ptr_range())
  {
    const auto & reference = libmesh_map_find(x0, node->id());
    auto & disp = libmesh_map_find(d, node->id());
    for (const auto i : make_range(Moose::dim))
      disp(i) = (*node)(i)-reference(i);
  }
}
