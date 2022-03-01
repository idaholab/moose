//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RinglebMesh.h"

#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"
#include "libmesh/mesh_modification.h"

registerMooseObject("MooseApp", RinglebMesh);

InputParameters
RinglebMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addRequiredParam<Real>("gamma", "Gamma parameter");
  params.addRequiredParam<Real>("kmax", "Value of k on the inner wall.");
  params.addRequiredParam<Real>("kmin", "Value of k on the outer wall.");
  params.addRequiredParam<int>("num_q_pts",
                               "How many points to discretize the range q = (0.5, k) into.");
  params.addRequiredParam<int>("n_extra_q_pts",
                               "How many 'extra' points should be inserted in the final element"
                               " *in addition to* the equispaced q points.");
  params.addRequiredParam<int>("num_k_pts", "How many points in the range k=(kmin, kmax).");
  params.addParam<boundary_id_type>("inflow_bid", 1, "The boundary id to use for the inflow");
  params.addParam<boundary_id_type>(
      "inner_wall_bid", 2, "The boundary id to use for the inner wall");
  params.addParam<boundary_id_type>("outflow_bid", 3, "The boundary id to use for the outflow");
  params.addParam<boundary_id_type>(
      "outer_wall_bid", 4, "The boundary id to use for the outer wall");
  params.addParam<bool>(
      "triangles", false, "If true, all the quadrilateral elements will be split into triangles");
  params.addClassDescription("Creates a mesh for the Ringleb problem.");

  return params;
}

RinglebMesh::RinglebMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _gamma(getParam<Real>("gamma")),
    _kmax(getParam<Real>("kmax")),
    _kmin(getParam<Real>("kmin")),
    _num_q_pts(getParam<int>("num_q_pts")),
    _n_extra_q_pts(getParam<int>("n_extra_q_pts")),
    _num_k_pts(getParam<int>("num_k_pts")),
    _inflow_bid(getParam<boundary_id_type>("inflow_bid")),
    _outflow_bid(getParam<boundary_id_type>("outflow_bid")),
    _inner_wall_bid(getParam<boundary_id_type>("inner_wall_bid")),
    _outer_wall_bid(getParam<boundary_id_type>("outer_wall_bid")),
    _triangles(getParam<bool>("triangles"))
{

  // catch likely user errors
  if (_kmax <= _kmin)
    mooseError("RinglebMesh: kmax must be greater than kmin");
}

std::unique_ptr<MooseMesh>
RinglebMesh::safeClone() const
{
  return std::make_unique<RinglebMesh>(*this);
}

std::vector<Real>
RinglebMesh::arhopj(const Real & gamma, const std::vector<Real> & q, const int & index)
{
  std::vector<Real> values(4);
  Real a = std::sqrt(1 - ((gamma - 1) / 2.) * std::pow(q[index], 2));
  Real rho = std::pow(a, 2. / (gamma - 1));
  Real p = (1. / gamma) * std::pow(a, 2 * gamma / (gamma - 1));
  Real J = 1. / a + 1. / (3. * std::pow(a, 3)) + 1. / (5. * std::pow(a, 5)) -
           0.5 * std::log((1 + a) / (1 - a));
  values = {a, rho, p, J};
  return values;
}

std::vector<Real>
RinglebMesh::computexy(const std::vector<Real> values,
                       const int & i,
                       const int & index,
                       const std::vector<Real> & ks,
                       const std::vector<Real> & q)
{
  std::vector<Real> xy(2);

  // Compute x(q,k)
  xy[0] = 0.5 / values[1] * (2. / ks[i] / ks[i] - 1. / q[index] / q[index]) - 0.5 * values[3];

  // Compute the term that goes under the sqrt sign
  // If 1 - (q/k)^2 is slightly negative, we make it zero.
  Real sqrt_term = 1. - q[index] * q[index] / ks[i] / ks[i];
  sqrt_term = std::max(sqrt_term, 0.);

  // Compute y(q,k)
  xy[1] = 1. / (ks[i] * values[1] * q[index]) * std::sqrt(sqrt_term);

  return xy;
}

void
RinglebMesh::buildMesh()
{
  MeshBase & mesh = getMesh();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  /// Data structure that holds pointers to the Nodes of each streamline.
  std::vector<std::vector<Node *>> stream_nodes(_num_k_pts);

  /// Node id counter.
  int current_node_id = 0;

  /// Vector containing the regularly spaced k values
  std::vector<Real> ks(_num_k_pts);
  Real diff = (_kmax - _kmin) / (_num_k_pts - 1);
  for (int i = 0; i < _num_k_pts; i++)
    ks[i] = _kmin + i * diff;

  for (int i = 0; i < _num_k_pts; i++)
  {
    stream_nodes[i].resize(2 * (_num_q_pts + _n_extra_q_pts));

    /// Vector containing the regularly spaced (and the extra q points) q values
    std::vector<Real> q(_num_q_pts);
    Real diffq = (ks[i] - 0.5) / (_num_q_pts - 1);
    for (int j = 0; j < _num_q_pts; j++)
      q[j] = 0.5 + j * diffq;

    /// Add the extra q points
    for (int j = _num_q_pts; j < _num_q_pts + _n_extra_q_pts; j++)
    {
      std::vector<Real>::iterator it = q.end();
      q.insert(--it, 0.3 * q[j - 2] + 0.7 * q[j - 1]);
    }

    std::vector<Real> vals(4);
    std::vector<Real> xy(2);
    /// Create the nodes for the upper part
    for (int j = 0; j < _num_q_pts + _n_extra_q_pts; j++)
    {
      // Compute the different parameters
      vals = arhopj(_gamma, q, j);

      // Compute x and y
      xy = computexy(vals, i, j, ks, q);

      // Create a node with (x,y) coordinates as it's on the upper part of the mesh
      if (j != _num_q_pts + _n_extra_q_pts - 1)
        stream_nodes[i][j] = mesh.add_point(Point(xy[0], xy[1]), current_node_id++);
    }

    /// Create the nodes for the lower part
    for (int j = _num_q_pts + _n_extra_q_pts; j < 2 * (_num_q_pts + _n_extra_q_pts); j++)
    {
      int index = 2 * (_num_q_pts + _n_extra_q_pts) - 1 - j;
      // Compute the different parameters
      vals = arhopj(_gamma, q, index);

      // Compute x and y
      xy = computexy(vals, i, index, ks, q);

      // Create a node with (x,-y) coordinates as it's on the lower part of the mesh
      stream_nodes[i][j] = mesh.add_point(Point(xy[0], -xy[1]), current_node_id++);
    }
  }

  /// Add elements for the whole mesh
  for (int i = 0; i < _num_k_pts - 1; i++)
  {
    for (int j = 0; j < 2 * (_num_q_pts + _n_extra_q_pts) - 1; j++)
    {
      /// This is done in order to avoid having two nodes at the exact same location (on the straight       /// line y=0)
      if (j != _num_q_pts + _n_extra_q_pts - 1 and j != _num_q_pts + _n_extra_q_pts - 2)
      {
        Elem * elem = mesh.add_elem(new Quad4);
        elem->set_node(0) = stream_nodes[i][j];
        elem->set_node(1) = stream_nodes[i][j + 1];
        elem->set_node(2) = stream_nodes[i + 1][j + 1];
        elem->set_node(3) = stream_nodes[i + 1][j];

        if (i == 0)
          boundary_info.add_side(elem->id(), /*side=*/0, _outer_wall_bid);
        if (j == 0)
          boundary_info.add_side(elem->id(), /*side=*/3, _inflow_bid);
        if (j == 2 * (_num_q_pts + _n_extra_q_pts) - 2)
          boundary_info.add_side(elem->id(), /*side=*/1, _outflow_bid);
        if (i == _num_k_pts - 2)
          boundary_info.add_side(elem->id(), /*side=*/2, _inner_wall_bid);
      }
      else if (j == _num_q_pts + _n_extra_q_pts - 2)
      {
        Elem * elem = mesh.add_elem(new Quad4);
        elem->set_node(0) = stream_nodes[i][j];
        elem->set_node(1) = stream_nodes[i][j + 2];
        elem->set_node(2) = stream_nodes[i + 1][j + 2];
        elem->set_node(3) = stream_nodes[i + 1][j];

        if (i == 0)
          boundary_info.add_side(elem->id(), /*side=*/0, _outer_wall_bid);
        if (i == _num_k_pts - 2)
          boundary_info.add_side(elem->id(), /*side=*/2, _inner_wall_bid);
      }
    }
  }

  /// Find neighbors, etc.
  mesh.prepare_for_use();

  /// Create the triangular elements if required by the user
  if (_triangles)
    MeshTools::Modification::all_tri(mesh);

  /// Create sideset names.
  boundary_info.sideset_name(_inflow_bid) = "inflow";
  boundary_info.sideset_name(_outflow_bid) = "outflow";
  boundary_info.sideset_name(_inner_wall_bid) = "inner_wall";
  boundary_info.sideset_name(_outer_wall_bid) = "outer_wall";
}
