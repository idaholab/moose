//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineSegment.h"
#include "MooseError.h"
#include "ParameterMesh.h"

#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/int_range.h"
#include "libmesh/dof_map.h"

#include "libmesh/elem.h"
#include "libmesh/face_quad.h"
#include "libmesh/face_quad4.h"
#include "libmesh/fe_compute_data.h"
#include "libmesh/fe_interface.h"
#include "libmesh/id_types.h"
#include "libmesh/int_range.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/explicit_system.h"
#include "libmesh/plane.h"
#include "libmesh/enum_to_string.h"
#include <memory>
#include "libmesh/quadrature_gauss.h"
#include "libmesh/fe_base.h"

using namespace libMesh;

ParameterMesh::ParameterMesh(const FEType & param_type,
                             const std::string & exodus_mesh,
                             const bool find_closest,
                             const unsigned int kdtree_candidates)
  : _communicator(MPI_COMM_SELF),
    _mesh(_communicator),
    _find_closest(find_closest),
    _kdtree_candidates(kdtree_candidates),
    _param_var_id(0),
    _dof_map(nullptr),
    _fe_type(param_type)
{
  _mesh.allow_renumbering(false);
  _mesh.prepare_for_use();
  _exodusII_io = std::make_unique<ExodusII_IO>(_mesh);
  _exodusII_io->read(exodus_mesh);
  _mesh.read(exodus_mesh);
  // Create system to store parameter values
  _eq = std::make_unique<EquationSystems>(_mesh);
  _sys = &_eq->add_system<ExplicitSystem>("_parameter_mesh_sys");
  _sys->add_variable("_parameter_mesh_var", param_type);

  // Create point locator
  _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, _mesh);
  _point_locator->enable_out_of_mesh_mode();

  // Initialize the equations systems
  _eq->init();

  // getting number of parameter dofs for size() function
  const unsigned short int var_id = _sys->variable_number("_parameter_mesh_var");
  std::set<dof_id_type> var_indices;
  _sys->local_dof_indices(var_id, var_indices);
  _param_dofs = var_indices.size();

  if (_find_closest)
  {
    for (const auto & elem : _mesh.element_ptr_range())
      if (elem->default_order() != FIRST)
        mooseError("Closet point projection currently does not support second order elements.");
  }

  // Initialize node-based KDTree optimization
  _mesh_nodes.clear();
  _node_to_elements.clear();

  // Extract all node coordinates
  for (const auto & node : _mesh.node_ptr_range())
    _mesh_nodes.push_back(*node);

  // Build node-to-elements connectivity map
  for (const auto & elem : _mesh.element_ptr_range())
  {
    for (const auto n : make_range(elem->n_nodes()))
    {
      dof_id_type node_id = elem->node_id(n);
      _node_to_elements[node_id].insert(elem);
    }
  }

  // Create KDTree from node coordinates
  if (!_mesh_nodes.empty())
    _node_kdtree = std::make_unique<KDTree>(_mesh_nodes, 10);
  // Update cached values for gradient computations
  const_cast<unsigned short int &>(_param_var_id) = var_id;
  const_cast<const DofMap *&>(_dof_map) = &_sys->get_dof_map();
  const_cast<FEType &>(_fe_type) = _dof_map->variable_type(_param_var_id);
}

void
ParameterMesh::getIndexAndWeight(const Point & pt,
                                 std::vector<dof_id_type> & dof_indices,
                                 std::vector<Real> & weights) const
{
  Point test_point = (_find_closest ? projectToMesh(pt) : pt);

  const Elem * elem = (*_point_locator)(test_point);
  if (!elem)
    mooseError("No element was found to contain point ", test_point);

  // Get the dof_indices for our element
  // variable id is hard coded to _param_var_id
  // this is probably the only variable in the ParameterMesh system used by ParameterMeshFunction
  _dof_map->dof_indices(elem, dof_indices, _param_var_id);

  // Map the physical co-ordinates to the reference co-ordinates
  Point coor = FEMap::inverse_map(elem->dim(), elem, test_point);
  // get the shape function value via the FEInterface
  FEComputeData fe_data(*_eq, coor);
  FEInterface::compute_data(elem->dim(), _fe_type, elem, fe_data);
  // Set weights to the value of the shape functions
  weights = fe_data.shape;

  if (dof_indices.size() != weights.size())
    mooseError("Internal error: weights and DoF indices do not have the same size.");
}

void
ParameterMesh::getIndexAndWeight(const Point & pt,
                                 std::vector<dof_id_type> & dof_indices,
                                 std::vector<RealGradient> & weights) const
{
  if (!_sys->has_variable("_parameter_mesh_var"))
    mooseError("Internal error: System being read does not contain _parameter_mesh_var.");
  Point test_point = (_find_closest ? projectToMesh(pt) : pt);
  // Locate the element the point is in
  const Elem * elem = (*_point_locator)(test_point);

  // Get the dof_indices for our element
  // variable id is hard coded to _param_var_id
  // this is probably the only variable in the ParameterMesh system used by ParameterMeshFunction
  _dof_map->dof_indices(elem, dof_indices, _param_var_id);

  // Map the physical co-ordinates to the reference co-ordinates
  Point coor = FEMap::inverse_map(elem->dim(), elem, test_point);
  // get the shape function value via the FEInterface
  FEComputeData fe_data(*_eq, coor);
  fe_data.enable_derivative();
  FEInterface::compute_data(elem->dim(), _fe_type, elem, fe_data);
  // Set weights to the value of the shape functions
  weights = fe_data.dshape;

  if (dof_indices.size() != weights.size())
    mooseError("Internal error: weights and DoF indices do not have the same size.");
}

Point
ParameterMesh::projectToMesh(const Point & p) const
{
  // quick path: p already inside an element
  if ((*_point_locator)(p))
    return p;

  // Lambda to find closest point from elements using squared distance for efficiency
  auto findClosestElement = [&p, this](const auto & elements) -> Point
  {
    Real best_d2 = std::numeric_limits<Real>::max();
    Point best_point = p;

    for (const auto * elem : elements)
    {
      Point trial = closestPoint(*elem, p);
      Real d2 = (trial - p).norm_sq();
      if (d2 < best_d2)
      {
        best_d2 = d2;
        best_point = trial;
      }
    }

    if (best_d2 == std::numeric_limits<Real>::max())
      mooseError("project_to_mesh failed - no candidate elements.");
    return best_point;
  };

  // Use KDTree optimization if available
  if (_node_kdtree && !_mesh_nodes.empty())
  {
    // Find K nearest nodes using KDTree
    std::vector<std::size_t> nearest_node_indices;
    _node_kdtree->neighborSearch(p, _kdtree_candidates, nearest_node_indices);

    // Collect all elements connected to these nodes
    std::set<const Elem *> candidate_elements;
    for (auto node_idx : nearest_node_indices)
    {
      // Get the actual node from the mesh using the index
      if (node_idx < _mesh.n_nodes())
      {
        const Node * node = _mesh.node_ptr(node_idx);
        dof_id_type node_id = node->id();
        auto it = _node_to_elements.find(node_id);
        if (it != _node_to_elements.end())
        {
          const auto & connected_elems = it->second;
          candidate_elements.insert(connected_elems.begin(), connected_elems.end());
        }
      }
    }

    // Convert set to vector for consistent type
    std::vector<const Elem *> candidate_vector(candidate_elements.begin(),
                                               candidate_elements.end());
    return findClosestElement(candidate_vector);
  }
  else
  {
    // Fallback to original O(n) method if KDTree not available
    std::vector<const Elem *> all_elements;
    for (const auto & elem : _mesh.element_ptr_range())
      all_elements.push_back(elem);

    return findClosestElement(all_elements);
  }
}

Point
ParameterMesh::closestPoint(const Elem & elem, const Point & p) const
{
  mooseAssert(!elem.contains_point(p),
              "Points inside of elements shouldn't need to find closestPoint.");

  // Lambda to find closest point from range without storing temporary vectors
  auto findClosest = [&p](auto range, auto point_func) -> Point
  {
    Real min_distance = std::numeric_limits<Real>::max();
    Point min_point = p;

    for (const auto & item : range)
    {
      Point candidate = point_func(item);
      Real distance = (candidate - p).norm();
      if (distance < min_distance)
      {
        min_distance = distance;
        min_point = candidate;
      }
    }
    return min_point;
  };

  switch (elem.type())
  {
    case EDGE2:
    {
      LineSegment ls(*(elem.node_ptr(0)), *(elem.node_ptr(1)));
      return ls.closest_point(p);
    }

    case TRI3:
    {
      Point a = *(elem.node_ptr(0));
      Point b = *(elem.node_ptr(1));
      Point c = *(elem.node_ptr(2));
      Plane pl(a, b, c);
      Point trial = pl.closest_point(p);
      if (elem.contains_point(trial))
        return trial;

      return findClosest(make_range(elem.n_edges()),
                         [&](dof_id_type i) { return closestPoint(*elem.build_edge_ptr(i), p); });
    }
    case QUAD4:
    {
      Point a = *(elem.node_ptr(0));
      Point b = *(elem.node_ptr(1));
      Point c = *(elem.node_ptr(2));
      Point d = *(elem.node_ptr(3));
      Plane pl1(a, b, c);
      Plane pl2(b, c, d);
      Point trial1 = pl1.closest_point(p);
      Point trial2 = pl2.closest_point(p);
      if (!trial1.absolute_fuzzy_equals(trial2, TOLERANCE * TOLERANCE))
        mooseError("Quad4 element is not coplanar");

      if (elem.contains_point(trial1))
        return trial1;

      return findClosest(make_range(elem.n_edges()),
                         [&](dof_id_type i) { return closestPoint(*elem.build_edge_ptr(i), p); });
    }

    default:
    {
      if (elem.dim() == 3)
      {
        return findClosest(make_range(elem.n_sides()),
                           [&](dof_id_type i) { return closestPoint(*elem.build_side_ptr(i), p); });
      }
      else
      {
        mooseError("Unsupported element type ",
                   Utility::enum_to_string(elem.type()),
                   " for projection of parameter mesh.");
      }
    }
  }
}

template <typename T>
T
ParameterMesh::computeRegularizationLoop(const std::vector<Real> & parameter_values,
                                         RegularizationType reg_type) const
{
  if (parameter_values.size() != _param_dofs)
    mooseError("Parameter values size (",
               parameter_values.size(),
               ") does not match mesh DOFs (",
               _param_dofs,
               ")");

  T result;
  if constexpr (std::is_same_v<T, Real>)
    result = 0.0;
  else if constexpr (std::is_same_v<T, std::vector<Real>>)
    result.resize(_param_dofs, 0.0);

  // Iterate over all elements in the mesh
  for (const auto & elem : _mesh.element_ptr_range())
  {
    // Get DOF indices for this element
    std::vector<dof_id_type> dof_indices;
    _dof_map->dof_indices(elem, dof_indices, _param_var_id);

    // Get quadrature rule for this element
    const unsigned int dim = elem->dim();
    QGauss qrule(dim, _fe_type.default_quadrature_order());

    // Create finite element objects
    std::unique_ptr<FEBase> fe(FEBase::build(dim, _fe_type));
    fe->attach_quadrature_rule(&qrule);

    // Request shape functions and derivatives before reinit
    const std::vector<Real> & JxW = fe->get_JxW();
    const std::vector<std::vector<Real>> & phi = fe->get_phi();
    const std::vector<std::vector<RealGradient>> & dphi = fe->get_dphi();

    // Reinitialize for current element
    fe->reinit(elem);

    for (const auto qp : make_range(qrule.n_points()))
    {
      if constexpr (std::is_same_v<T, Real>)
        result +=
            computeRegularizationQp(parameter_values, phi, dphi, qp, dof_indices, JxW, reg_type);
      else if constexpr (std::is_same_v<T, std::vector<Real>>)
        computeRegularizationGradientQp(
            parameter_values, phi, dphi, qp, dof_indices, JxW, reg_type, result);
    }
  }

  return result;
}

Real
ParameterMesh::computeRegularizationObjective(const std::vector<Real> & parameter_values,
                                              RegularizationType reg_type) const
{
  return computeRegularizationLoop<Real>(parameter_values, reg_type);
}

std::vector<Real>
ParameterMesh::computeRegularizationGradient(const std::vector<Real> & parameter_values,
                                             RegularizationType reg_type) const
{
  return computeRegularizationLoop<std::vector<Real>>(parameter_values, reg_type);
}

Real
ParameterMesh::computeRegularizationQp(const std::vector<Real> & parameter_values,
                                       const std::vector<std::vector<Real>> & /*phi*/,
                                       const std::vector<std::vector<RealGradient>> & dphi,
                                       const unsigned int qp,
                                       const std::vector<dof_id_type> & dof_indices,
                                       const std::vector<Real> & JxW,
                                       RegularizationType reg_type) const
{
  Real objective_contribution = 0.0;

  // Switch on regularization type
  switch (reg_type)
  {
    case RegularizationType::L2_GRADIENT:
    {
      // Compute parameter gradient at this quadrature point
      RealGradient param_grad;
      for (const auto i : index_range(dof_indices))
        param_grad += parameter_values[dof_indices[i]] * dphi[i][qp];

      // Add L2 norm squared of gradient for regularization
      objective_contribution = param_grad.norm_sq() * JxW[qp];
      break;
    }
    default:
      mooseError("Unknown Regularization Type");
  }

  return objective_contribution;
}

void
ParameterMesh::computeRegularizationGradientQp(const std::vector<Real> & parameter_values,
                                               const std::vector<std::vector<Real>> & /*phi*/,
                                               const std::vector<std::vector<RealGradient>> & dphi,
                                               const unsigned int qp,
                                               const std::vector<dof_id_type> & dof_indices,
                                               const std::vector<Real> & JxW,
                                               RegularizationType reg_type,
                                               std::vector<Real> & gradient) const
{
  // Switch on regularization type
  switch (reg_type)
  {
    case RegularizationType::L2_GRADIENT:
    {
      // Compute parameter gradient at this quadrature point
      RealGradient param_grad;
      for (const auto i : index_range(dof_indices))
        param_grad += parameter_values[dof_indices[i]] * dphi[i][qp];

      // Compute gradient contribution: 2 * grad(p) * dphi_j
      for (const auto j : index_range(dof_indices))
        gradient[dof_indices[j]] += 2.0 * param_grad * dphi[j][qp] * JxW[qp];
      break;
    }

    default:
      mooseError("Unknown Regularization Type");
  }
}
