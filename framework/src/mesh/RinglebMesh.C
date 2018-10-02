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

registerMooseObject("MooseApp", RinglebMesh);

template <>
InputParameters
validParams<RinglebMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addRequiredParam<Real>("gamma", "Gamma parameter");
  params.addRequiredParam<Real>("kmax", "Value of k on the inner wall.");
  params.addRequiredParam<Real>("kmin", "Value of k on the outer wall.");
  params.addRequiredParam<int>("num_q_pts", "How many points to discretize the range q = (0.5, k) into.");
  params.addRequiredParam<int>("n_extra_q_pts",
                               "How many 'extra' points should be inserted in the final element"
                               " *in addition to* the equispaced q points.");
  params.addRequiredParam<int>("num_k_pts", "How many points in the range k=(kmin, kmax).");
  params.addParam<boundary_id_type>("inflow_bid", 1, "The boundary id to use for the inflow");
  params.addParam<boundary_id_type>("inner_wall_bid", 2, "The boundary id to use for the inner wall");
  params.addParam<boundary_id_type>("outflow_bid", 3, "The boundary id to use for the outflow");
  params.addParam<boundary_id_type>("outer_wall_bid", 4, "The boundary id to use for the outer wall");
  params.addParam<bool>("triangles", false,
                        "If true, all the quadrilateral elements will be split into triangles");
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
  return libmesh_make_unique<RinglebMesh>(*this);
}

void
RinglebMesh::buildMesh()
{
  MeshBase & mesh = getMesh();

  // Data structure that holds pointers to the Nodes of the upper part of each streamline.
  std::vector<std::vector<Node *>> stream_nodes_up(_num_k_pts);

  // Node id counter.
  int current_node_id = 0;

  // Vector containing the regularly spaced k values
  std::vector<Real> ks(_num_k_pts);
  Real diff = (_kmax-_kmin)/(_num_k_pts-1);
  for (int i = 0; i < _num_k_pts; i++)
    ks[i] = _kmin+i*diff;

  for (int i = 0; i < _num_k_pts; i++)
  {
    stream_nodes_up[i].resize(2 * (_num_q_pts + _n_extra_q_pts));

    //Vector containing the regularly spaced (and the extra q points) q values
    std::vector<Real> q(_num_q_pts);
    Real diffq = (ks[i] - 0.5)/(_num_q_pts - 1);
    for (int j = 0; j < _num_q_pts; j++)
      q[j] = 0.5 + j * diffq;

    // Add the extra q points
    for (int j = _num_q_pts; j < _num_q_pts + _n_extra_q_pts; j++)
    {
      std::vector<Real>::iterator it = q.end();
      q.insert(--it,  0.3 * q[j - 2] + 0.7 * q[j - 1]);
    }

    Real a, rho, p, J, x, y, sqrt_term;
    // Up
    for (int j = 0; j < _num_q_pts + _n_extra_q_pts; j++)
    {
      // Compute the different parameters
      a = std::sqrt(1 - ((_gamma - 1)/2) * std::pow(q[j],2));
      rho = std::pow(a, 2. / (_gamma - 1));
      p = (1./_gamma) * std::pow(a, 2 * _gamma / (_gamma - 1));
      J = 1. / a + 1. /(3. * std::pow(a,3)) + 1. / (5. * std::pow(a,5)) - 0.5 * std::log( (1+a) / (1-a) );

      // Compute x(q,k)
      x = 0.5 / rho * (2. / ks[i] / ks[i] - 1. / q[j] / q[j]) - 0.5 * J;

      // Compute the term that goes under the sqrt sign
      // If 1 - (q/k)^2 is slightly negative, we make it zero.
      sqrt_term = 1. - q[j] * q[j] / ks[i] / ks[i];
      sqrt_term = std::max(sqrt_term, 0.);

      // Compute y(q,k)
      y = 1. / (ks[i] * rho * q[j]) * std::sqrt(sqrt_term);

      // Create a node with (x,y) coordinates if it's on the upper part of the mesh
      // (x,-y) if it's on the lower part

      stream_nodes_up[i][j] = mesh.add_point(Point(x,y), current_node_id++);
      
    }

    // Low
    for (int j = _num_q_pts + _n_extra_q_pts; j < 2 * (_num_q_pts + _n_extra_q_pts); j++)
    {
      // Compute the different parameters
      a = std::sqrt(1 - ((_gamma - 1)/2) * std::pow(q[2 * (_num_q_pts + _n_extra_q_pts) - 1 - j],2));
      rho = std::pow(a, 2. / (_gamma - 1));
      p = (1./_gamma) * std::pow(a, 2 * _gamma / (_gamma - 1));
      J = 1. / a + 1. /(3. * std::pow(a,3)) + 1. / (5. * std::pow(a,5)) - 0.5 * std::log( (1+a) / (1-a) );

      // Compute x(q,k)
      x = 0.5 / rho * (2. / ks[i] / ks[i] - 1. / q[2 * (_num_q_pts + _n_extra_q_pts) - 1 - j] / q[2 * (_num_q_pts + _n_extra_q_pts) - 1 - j]) - 0.5 * J;

      // Compute the term that goes under the sqrt sign
      // If 1 - (q/k)^2 is slightly negative, we make it zero.
      sqrt_term = 1. - q[2 * (_num_q_pts + _n_extra_q_pts) - 1 - j] * q[2 * (_num_q_pts + _n_extra_q_pts) - 1 - j] / ks[i] / ks[i];
      sqrt_term = std::max(sqrt_term, 0.);

      // Compute y(q,k)
      y = 1. / (ks[i] * rho * q[2 * (_num_q_pts + _n_extra_q_pts) - 1 - j]) * std::sqrt(sqrt_term);

      // Create a node with (x,y) coordinates if it's on the upper part of the mesh
      // (x,-y) if it's on the lower part

      stream_nodes_up[i][j] = mesh.add_point(Point(x,-y), current_node_id++);
      
    }
  }

  // Add elements
  for (int i = 0; i < _num_k_pts - 1; i++)
  {
    for (int j = 0; j < 2 * ( _num_q_pts + _n_extra_q_pts) - 1; j++)
    {
      Elem * elem = mesh.add_elem(new Quad4);
      elem->set_node(0) = stream_nodes_up[i][j];
      elem->set_node(1) = stream_nodes_up[i][j + 1];
      elem->set_node(2) = stream_nodes_up[i + 1][j + 1];
      elem->set_node(3) = stream_nodes_up[i + 1][j];

      if (i == 0)
        mesh.boundary_info->add_side(elem->id(), /*side=*/0, _outer_wall_bid);
      if (j == 0)
        mesh.boundary_info->add_side(elem->id(), /*side=*/3, _inflow_bid);
      if (j == 2 * (_num_q_pts + _n_extra_q_pts) - 2)
        mesh.boundary_info->add_side(elem->id(), /*side=*/1, _outflow_bid);
      if (i == _num_k_pts -2)
        mesh.boundary_info->add_side(elem->id(), /*side=*/2, _inner_wall_bid);
    }
  }

  // Find neighbors, etc.
  mesh.prepare_for_use();

  if (_triangles)
    MeshTools::Modification::all_tri(mesh);

  // Create sideset names.
  mesh.boundary_info->sideset_name(_inflow_bid) = "inflow";
  mesh.boundary_info->sideset_name(_outflow_bid) = "outflow";
  mesh.boundary_info->sideset_name(_inner_wall_bid) = "inner_wall";
  mesh.boundary_info->sideset_name(_outer_wall_bid) = "outer_wall";
}
