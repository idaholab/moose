//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSideSetsBase.h"
#include "Parser.h"
#include "InputParameters.h"
#include "MooseMesh.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"

template <>
InputParameters
validParams<AddSideSetsBase>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addParam<Real>(
      "variance", 0.10, "The variance [0.0 - 1.0] allowed when comparing normals");
  params.addParam<bool>("fixed_normal",
                        false,
                        "This Boolean determines whether we fix our normal "
                        "or allow it to vary to \"paint\" around curves");

  return params;
}

AddSideSetsBase::AddSideSetsBase(const InputParameters & parameters)
  : MeshModifier(parameters),
    _variance(getParam<Real>("variance")),
    _fixed_normal(getParam<bool>("fixed_normal"))
{
}

AddSideSetsBase::~AddSideSetsBase() {}

void
AddSideSetsBase::setup()
{
  mooseAssert(_mesh_ptr, "Mesh pointer is NULL");
  mooseAssert(_fe_face == nullptr, "FE Face has already been initialized");

  unsigned int dim = _mesh_ptr->dimension();

  // Setup the FE Object so we can calculate normals
  FEType fe_type(Utility::string_to_enum<Order>("CONSTANT"),
                 Utility::string_to_enum<FEFamily>("MONOMIAL"));
  _fe_face = FEBase::build(dim, fe_type);
  _qface = libmesh_make_unique<QGauss>(dim - 1, FIRST);
  _fe_face->attach_quadrature_rule(_qface.get());
}

void
AddSideSetsBase::finalize()
{
  _qface.reset();
  _fe_face.reset();
}

void
AddSideSetsBase::flood(const Elem * elem, Point normal, BoundaryID side_id)
{
  if (elem == nullptr || (_visited[side_id].find(elem) != _visited[side_id].end()))
    return;

  _visited[side_id].insert(elem);
  for (unsigned int side = 0; side < elem->n_sides(); ++side)
  {
    if (elem->neighbor_ptr(side))
      continue;

    _fe_face->reinit(elem, side);
    const std::vector<Point> normals = _fe_face->get_normals();

    // We'll just use the normal of the first qp
    if (std::abs(1.0 - normal * normals[0]) <= _variance)
    {
      _mesh_ptr->getMesh().get_boundary_info().add_side(elem, side, side_id);
      for (unsigned int neighbor = 0; neighbor < elem->n_sides(); ++neighbor)
      {
        // Flood to the neighboring elements using the current matching side normal from this
        // element.
        // This will allow us to tolerate small changes in the normals so we can "paint" around a
        // curve.
        flood(elem->neighbor_ptr(neighbor), _fixed_normal ? normal : normals[0], side_id);
      }
    }
  }
}
