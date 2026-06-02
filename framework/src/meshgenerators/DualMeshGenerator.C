//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualMeshGenerator.h"
#include "Conversion.h"
#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", DualMeshGenerator);

InputParameters
DualMeshGenerator::validParams()
{
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Takes a 2D mesh as input and returns a Voronoi dual mesh, i.e.,"
                             "changes each input mode into an element and each input element "
                             "into a node located at its centroid.");
  params.addRequiredParam<RealVectorValue>(
      "bottom_left", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<RealVectorValue>(
      "top_right", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<subdomain_id_type>(
      "block_id", "Subdomain id to set for inside/outside the bounding box");
  params.addParam<SubdomainName>(
      "block_name", "Subdomain name to set for inside/outside the bounding box (optional)");
  params.addParam<MooseEnum>(
      "location", location, "Control of where the subdomain id is to be set");
  params.addParam<std::vector<SubdomainName>>(
      "restricted_subdomains",
      "Only reset subdomain ID for given subdomains within the bounding box");

  params.addParam<std::string>("integer_name",
                               "Element integer to be assigned (default to subdomain ID)");
  return params;
}

DualMeshGenerator::DualMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _location(parameters.get<MooseEnum>("location")),
    _block_id(parameters.get<subdomain_id_type>("block_id")),
    _has_restriction(isParamValid("restricted_subdomains")),
    _bounding_box(MooseUtils::buildBoundingBox(parameters.get<RealVectorValue>("bottom_left"),
                                               parameters.get<RealVectorValue>("top_right")))
{
}

std::unique_ptr<MeshBase>
DualMeshGenerator::generate()
{
  const auto mesh = std::move(_input);
  std::vector<libMesh::Point> centroids; // vector of all dual nodes
  centroids.reserve(mesh->n_elem());

  std::unordered_map<dof_id_type, std::vector<dof_id_type>> _node_to_elem_map;

  unsigned int i;

  //////////Adding nodes//////
  for (const auto & in_elem : mesh->element_ptr_range())
  {

    _console << "Found centroid: " << in_elem->true_centroid() << std::endl;

    Point centroid = in_elem->true_centroid();
    centroids.push_back(centroid);
    for (unsigned int n = 0; n < in_elem->n_nodes(); n++)
    {
      _node_to_elem_map[in_elem->node_id(n)].push_back(in_elem->id());
    }
  }

  auto dualMesh = buildReplicatedMesh(mesh->mesh_dimension());

  for (i = 0; i < centroids.size(); ++i)
  {
    dualMesh->add_point(centroids[i]);
  }

  std::unordered_map<dof_id_type, std::vector<dof_id_type>> _elem_to_node_map;
  for (const auto & [node_id, elements] : _node_to_elem_map)
  {
    for (dof_id_type elem_id : elements)
    {
      _elem_to_node_map[elem_id].push_back(node_id);
    }
  } //_elem_to_node_map now has element IDs in the first entry and maps to the nodes that make
    // up that element

  _console << "Mesh populated with dual nodes" << std::endl;

  // Get what nodes are boundary nodes
  // looping over primal elements....
  std::unordered_map<dof_id_type, std::vector<Point>> node_to_boundary_midpoints;
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> node_to_boundary_neighbors;
  for (const auto & primalElem : mesh->element_ptr_range())
  {
    // looping over each side
    for (const auto & side : primalElem->side_index_range())
    {
      if (primalElem->neighbor_ptr(side) == nullptr) // if its a boundary side..
      {
        std::unique_ptr<Elem> side_elem = primalElem->build_side_ptr(side);

        std::vector<Node *> side_nodes;
        _console << "found bordering side with nodes: " << std::endl;
        for (unsigned int i = 0; i < side_elem->n_nodes(); ++i)
        {
          Node * node = side_elem->node_ptr(i);
          node->print_info();
          side_nodes.push_back(node);
        }

        // find midpoint.
        Point midPoint;
        for (const auto & node : side_nodes)
        {
          midPoint += *node;
        }
        midPoint /= side_nodes.size();

        for (auto * node : side_nodes)
          node_to_boundary_midpoints[node->id()].push_back(midPoint);

        // Recording boundary edge connectivity
        if (side_nodes.size() == 2)
        {
          const auto id0 = side_nodes[0]->id();
          const auto id1 = side_nodes[1]->id();

          node_to_boundary_neighbors[id0].push_back(id1);
          node_to_boundary_neighbors[id1].push_back(id0);
        }
      }
    }
    //_console << "Looking at next Element..." << std::endl;
  }
  // boundaryMidPoints now contains all of the node pointers that are boundary nodes that might need
  // to be added to a dual mesh.
  // Boundary Helper
  auto isBoundaryVertex = [&](dof_id_type node_id) -> bool
  {
    auto it = node_to_boundary_neighbors.find(node_id);
    if (it == node_to_boundary_neighbors.end())
      return false;

    auto neighbors = it->second;

    std::sort(neighbors.begin(), neighbors.end());
    neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());

    if (neighbors.size() != 2)
      return false;

    const Point & p = *mesh->node_ptr(node_id);
    Point v0 = *mesh->node_ptr(neighbors[0]) - p;
    Point v1 = *mesh->node_ptr(neighbors[1]) - p;

    const Real n0 = v0.norm();
    const Real n1 = v1.norm();

    if (n0 == 0.0 || n1 == 0.0)
      return false;

    Real c = (v0 * v1) / (n0 * n1);
    c = std::max(Real(-1), std::min(Real(1), c));

    const Real angle_deg = std::acos(c) * 180.0 / libMesh::pi;

    return std::abs(angle_deg - 180.0) > 1.0;
  };

  // loop over all primal nodes / dual elements
  for (const auto & [primalNodeID, primalElemIDs] : _node_to_elem_map)
  {
    _console << "Number of nodes for dual element: " << primalElemIDs.size() << std::endl;

    if (primalElemIDs.size() >= 3)
    {

      _console << "Loading interor polygon!" << std::endl;

      // Define a dual element located at each primal node
      std::unique_ptr<Elem> dualElem = std::make_unique<libMesh::C0Polygon>(primalElemIDs.size());

      // Now loop over the # of nodes on each dual element
      std::vector<std::pair<Node *, Real>> dualNodesAndPhis;
      auto primalNode = mesh->node_ptr(primalNodeID);

      for (unsigned int j = 0; j < primalElemIDs.size();
           ++j) // n.second.size is number of dual nodes to the dual element
      {
        const dof_id_type dualNodeOnPElem_id =
            primalElemIDs[j]; // Grab the dual nodes' IDs on each primal element

        auto const dualNodeOnPElem = dualMesh->node_ptr(
            dualNodeOnPElem_id); // Grab the dual nodes corresponding to these IDs

        Real dualNodeX = (dualNodeOnPElem->operator()(0)) - primalNode->operator()(0);
        Real dualNodeY = (dualNodeOnPElem->operator()(1)) - primalNode->operator()(1);
        Real nodePhi = atan2(dualNodeY, dualNodeX);

        dualNodesAndPhis.push_back({dualNodeOnPElem, nodePhi});

        std::sort(dualNodesAndPhis.begin(),
                  dualNodesAndPhis.end(),
                  [](const auto & a, const auto & b) { return a.second < b.second; });
      }
      for (unsigned int k = 0; k < dualNodesAndPhis.size(); ++k)
      {
        dualElem->set_node(k,
                           dualNodesAndPhis[k].first); // assign these nodes to the the dual element
      }

      // add the element to the mesh, now that it's assigned nodes
      dualMesh->add_elem(std::move(dualElem));
    }
    else
    {

      std::vector<std::pair<Node *, Real>> dualNodesAndPhis;
      Node * primalNode = mesh->node_ptr(primalNodeID);

      if (isBoundaryVertex(primalNodeID))
      {
        Node * cornerNode = dualMesh->add_point(*mesh->node_ptr(primalNodeID));

        dualNodesAndPhis.push_back({cornerNode, 0.0});
      }

      // loop over all nodes, the loop over all elements in the primal mesh.

      // if for this node, it belongs to only one element, add it to dualNodesAndPhis

      // Add centroid nodes from adjacent primal elements
      for (const auto elem_id : primalElemIDs)
      {
        Node * dualNode = dualMesh->node_ptr(elem_id);

        Real dx = (*dualNode)(0) - (*primalNode)(0);
        Real dy = (*dualNode)(1) - (*primalNode)(1);

        dualNodesAndPhis.push_back({dualNode, std::atan2(dy, dx)});
      }

      // Add boundary midpoint nodes
      for (const auto & midpoint : node_to_boundary_midpoints[primalNodeID])
      {
        Node * midpointNode = dualMesh->add_point(midpoint);

        Real dx = midpoint(0) - (*primalNode)(0);
        Real dy = midpoint(1) - (*primalNode)(1);

        dualNodesAndPhis.push_back({midpointNode, std::atan2(dy, dx)});
      }

      // Recompute angles around the geometric center of corner dual elements.

      Point center;

      for (const auto & [node, phi] : dualNodesAndPhis)
        center += *node;

      center /= dualNodesAndPhis.size();

      for (auto & [node, phi] : dualNodesAndPhis)
      {
        phi = std::atan2((*node)(1) - center(1), (*node)(0) - center(0));
      }

      std::sort(dualNodesAndPhis.begin(),
                dualNodesAndPhis.end(),
                [](const auto & a, const auto & b) { return a.second < b.second; });

      if (dualNodesAndPhis.size() >= 3)
      {
        std::unique_ptr<Elem> dualElem =
            std::make_unique<libMesh::C0Polygon>(dualNodesAndPhis.size());

        for (unsigned int k = 0; k < dualNodesAndPhis.size(); ++k)
          dualElem->set_node(k, dualNodesAndPhis[k].first);

        dualMesh->add_elem(std::move(dualElem));
      }
    }
  }

  dualMesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(dualMesh);
}