//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeleteElementsNearMeshGenerator.h"
#include "SetupQuadratureAction.h"

#include "libmesh/type_vector.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#include "KDTree.h"

registerMooseObject("MooseApp", DeleteElementsNearMeshGenerator);

InputParameters
DeleteElementsNearMeshGenerator::validParams()
{
  InputParameters params = ElementDeletionGeneratorBase::validParams();

  params.addClassDescription(
      "Removes elements lying 'near' another mesh. The proximity is examined by the distance from "
      "the element's centroid to the faces of elements of the 'proximity_mesh'");
  params.addRequiredParam<MeshGeneratorName>("proximity_mesh",
                                             "Mesh providing the deletion criterion");
  params.addRequiredRangeCheckedParam<Real>(
      "distance",
      "distance>0",
      "The distance from the centroid of elements in the 'input' mesh to elements in the "
      "'proximity_mesh' under which they are marked for deletion");
  auto options_order = SetupQuadratureAction::getQuadratureOrderEnum();
  options_order.assign(CONSTANT);
  params.addParam<MooseEnum>(
      "side_order",
      options_order,
      "Order of the face quadrature used to find the nearest face in the 'proximity_mesh'");

  return params;
}

DeleteElementsNearMeshGenerator::DeleteElementsNearMeshGenerator(const InputParameters & parameters)
  : ElementDeletionGeneratorBase(parameters),
    _proximity_mesh(getMesh("proximity_mesh")),
    _distance(getParam<Real>("distance"))
{
}

std::unique_ptr<MeshBase>
DeleteElementsNearMeshGenerator::generate()
{
  // Build the point locator to detect elements inside
  _pl = _proximity_mesh->sub_point_locator();
  _pl->enable_out_of_mesh_mode();

  // Build a KNN with side Qps. This will help locate the nearest side if known to be outside
  // NOTE: side Qps are just one option. We could have done nodes, side centroids (~ side Qp at
  // order 0)
  std::vector<Point> all_side_qps;
  const auto order = getParam<MooseEnum>("side_order");
  all_side_qps.reserve(_proximity_mesh->n_elem() * _proximity_mesh->spatial_dimension() * 2 *
                       order);
  for (const auto elem : _proximity_mesh->element_ptr_range())
  {
    // Build side then side Qps
    unsigned int dim = elem->dim();
    for (const auto side_i : make_range(elem->n_sides()))
    {
      // Internal sides cannot be closest to an external point
      if (elem->neighbor_ptr(side_i))
        continue;
      const std::unique_ptr<const Elem> face = elem->build_side_ptr(side_i);
      std::unique_ptr<libMesh::FEBase> fe(
          libMesh::FEBase::build(dim, libMesh::FEType(elem->default_order())));
      libMesh::QGauss qface(dim - 1, Moose::stringToEnum<Order>(order));
      fe->attach_quadrature_rule(&qface);
      const std::vector<libMesh::Point> & qpoints = fe->get_xyz();
      fe->reinit(elem, side_i);
      all_side_qps.insert(all_side_qps.end(), qpoints.begin(), qpoints.end());
    }
  }
  mooseAssert(!all_side_qps.empty(), "Should have found side Qps");
  _kd_tree = std::make_unique<KDTree>(all_side_qps, /*max leaf size*/ 1);

  // Abide by the rules
  // NOTE: don't use _proximity_mesh in shouldDelete()! It won't work, it's gone
  const auto prox_mg = std::move(_proximity_mesh);

  // Perform the deletions
  return ElementDeletionGeneratorBase::generate();
}

bool
DeleteElementsNearMeshGenerator::shouldDelete(const Elem * elem)
{
  const auto delete_due_to_point = [this](const Point & pt) -> bool
  {
    // Proximity mesh contains the point, distance is zero
    if ((*_pl)(pt))
      return true;

    // Use the KNN to get the distance
    std::vector<Real> distance_sqr(2);
    std::vector<std::size_t> return_indices(2);
    _kd_tree->neighborSearch(pt, /*num_search*/ 1, return_indices, distance_sqr);
    const auto distance =
        distance_sqr.empty() ? std::numeric_limits<Real>::max() : std::sqrt(distance_sqr[0]);

    return distance < _distance;
  };

  // Check element centroid
  const auto centroid = elem->vertex_average();
  if (delete_due_to_point(centroid))
    return true;

  // Then its nodes. For convex elements, this should be enough
  for (const auto & node : elem->node_ref_range())
    if (delete_due_to_point(node))
      return true;

  return false;
}
