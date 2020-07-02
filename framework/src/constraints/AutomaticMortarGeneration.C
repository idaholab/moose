//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AutomaticMortarGeneration.h"
#include "MortarSegmentInfo.h"
#include "NanoflannMeshAdaptor.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseLagrangeHelpers.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/explicit_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/dof_map.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/quadrature_trap.h"

#include "metaphysicl/dualnumber.h"

#include <array>

using namespace libMesh;
using MetaPhysicL::DualNumber;

const std::string AutomaticMortarGeneration::system_name = "Nodal Normals";

AutomaticMortarGeneration::AutomaticMortarGeneration(
    MooseApp & app,
    MeshBase & mesh_in,
    const std::pair<BoundaryID, BoundaryID> & boundary_key,
    const std::pair<SubdomainID, SubdomainID> & subdomain_key,
    bool on_displaced,
    bool periodic)
  : ConsoleStreamInterface(app),
    mesh(mesh_in),
    mortar_segment_mesh(mesh_in.comm()),
    h_max(0.),
    _debug(false),
    _on_displaced(on_displaced),
    _periodic(periodic)
{
  primary_secondary_boundary_id_pairs.push_back(boundary_key);
  primary_requested_boundary_ids.insert(boundary_key.first);
  secondary_requested_boundary_ids.insert(boundary_key.second);
  primary_secondary_subdomain_id_pairs.push_back(subdomain_key);
  primary_boundary_subdomain_ids.insert(subdomain_key.first);
  secondary_boundary_subdomain_ids.insert(subdomain_key.second);
}

void
AutomaticMortarGeneration::clear()
{
  mortar_segment_mesh.clear();
  nodes_to_secondary_elem_map.clear();
  nodes_to_primary_elem_map.clear();
  secondary_node_and_elem_to_xi2_primary_elem.clear();
  primary_node_and_elem_to_xi1_secondary_elem.clear();
  msm_elem_to_info.clear();
  lower_elem_to_side_id.clear();
  mortar_interface_coupling.clear();
  secondary_node_to_nodal_normal.clear();
}

void
AutomaticMortarGeneration::buildNodeToElemMaps()
{
  if (secondary_requested_boundary_ids.empty() || primary_requested_boundary_ids.empty())
    mooseError(
        "Must specify secondary and primary boundary ids before building node-to-elem maps.");

  // Construct nodes_to_secondary_elem_map
  for (const auto & secondary_elem :
       as_range(mesh.active_elements_begin(), mesh.active_elements_end()))
  {
    // If this is not one of the lower-dimensional secondary side elements, go on to the next one.
    if (!this->secondary_boundary_subdomain_ids.count(secondary_elem->subdomain_id()))
      continue;

    for (MooseIndex(secondary_elem->n_vertices()) n = 0; n < secondary_elem->n_vertices(); ++n)
    {
      std::vector<const Elem *> & vec = nodes_to_secondary_elem_map[secondary_elem->node_id(n)];
      vec.push_back(secondary_elem);
    }
  }

  // Construct nodes_to_primary_elem_map
  for (const auto & primary_elem :
       as_range(mesh.active_elements_begin(), mesh.active_elements_end()))
  {
    // If this is not one of the lower-dimensional primary side elements, go on to the next one.
    if (!this->primary_boundary_subdomain_ids.count(primary_elem->subdomain_id()))
      continue;

    for (MooseIndex(primary_elem->n_vertices()) n = 0; n < primary_elem->n_vertices(); ++n)
    {
      std::vector<const Elem *> & vec = nodes_to_primary_elem_map[primary_elem->node_id(n)];
      vec.push_back(primary_elem);
    }
  }
}

void
AutomaticMortarGeneration::buildMortarSegmentMesh()
{
  // We maintain a mapping from lower-dimensional secondary elements in
  // the original mesh to (sets of) elements in mortar_segment_mesh.
  // This allows us to quickly determine which elements need to be
  // split.
  std::map<const Elem *, std::set<Elem *>> secondary_elems_to_mortar_segments;

  // 1.) Add all lower-dimensional secondary side elements as the "initial" mortar segments.
  for (MeshBase::const_element_iterator el = mesh.active_elements_begin(),
                                        end_el = mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * secondary_elem = *el;

    // If this is not one of the lower-dimensional secondary side elements, go on to the next one.
    if (!this->secondary_boundary_subdomain_ids.count(secondary_elem->subdomain_id()))
      continue;

    std::vector<Node *> new_nodes;
    for (MooseIndex(secondary_elem->n_nodes()) n = 0; n < secondary_elem->n_nodes(); ++n)
      new_nodes.push_back(
          mortar_segment_mesh.add_point(secondary_elem->point(n), secondary_elem->node_id(n)));

    Elem * new_elem;
    if (secondary_elem->default_order() == SECOND)
      new_elem = mortar_segment_mesh.add_elem(new Edge3);
    else
      new_elem = mortar_segment_mesh.add_elem(new Edge2);

    new_elem->processor_id() = secondary_elem->processor_id();
    new_elem->set_parent(const_cast<Elem *>(secondary_elem->parent()));
    new_elem->set_interior_parent(const_cast<Elem *>(secondary_elem->interior_parent()));

    for (MooseIndex(new_elem->n_nodes()) n = 0; n < new_elem->n_nodes(); ++n)
      new_elem->set_node(n) = new_nodes[n];

    // The xi^(1) values for this mortar segment are initially -1 and 1.
    MortarSegmentInfo msinfo;
    msinfo.xi1_a = -1;
    msinfo.xi1_b = +1;
    msinfo.secondary_elem = secondary_elem;

    auto new_container_it0 = secondary_node_and_elem_to_xi2_primary_elem.find(
             std::make_pair(secondary_elem->node_ptr(0), secondary_elem)),
         new_container_it1 = secondary_node_and_elem_to_xi2_primary_elem.find(
             std::make_pair(secondary_elem->node_ptr(1), secondary_elem));

    bool new_container_node0_found =
             (new_container_it0 != secondary_node_and_elem_to_xi2_primary_elem.end()),
         new_container_node1_found =
             (new_container_it1 != secondary_node_and_elem_to_xi2_primary_elem.end());

    const Elem * node0_primary_candidate = nullptr;
    const Elem * node1_primary_candidate = nullptr;

    if (new_container_node0_found)
    {
      const auto & xi2_primary_elem_pair = new_container_it0->second;
      msinfo.xi2_a = xi2_primary_elem_pair.first;
      node0_primary_candidate = xi2_primary_elem_pair.second;
    }

    if (new_container_node1_found)
    {
      const auto & xi2_primary_elem_pair = new_container_it1->second;
      msinfo.xi2_b = xi2_primary_elem_pair.first;
      node1_primary_candidate = xi2_primary_elem_pair.second;
    }

    // If both node0 and node1 agree on the primary element they are
    // projected into, then this mortar segment fits entirely within
    // a single primary element, and we can go ahead and set the
    // msinfo.primary_elem pointer now.
    if (node0_primary_candidate == node1_primary_candidate)
      msinfo.primary_elem = node0_primary_candidate;

    // Associate this MSM elem with the MortarSegmentInfo.
    msm_elem_to_info.insert(std::make_pair(new_elem, msinfo));

    // Maintain the mapping between secondary elems and mortar segment elems contained within them.
    // Initially, only the original secondary_elem is present.
    secondary_elems_to_mortar_segments[secondary_elem].insert(new_elem);
  }

  // 2.) Insert new nodes from primary side and split mortar segments as necessary.
  for (const auto & pr : primary_node_and_elem_to_xi1_secondary_elem)
  {
    auto key = pr.first;
    auto val = pr.second;

    const Node * primary_node = std::get<1>(key);
    Real xi1 = val.first;
    const Elem * secondary_elem = val.second;

    // If this is an aligned node, we don't need to do anything.
    if (std::abs(std::abs(xi1) - 1.) < TOLERANCE)
      continue;

    auto && order = secondary_elem->default_order();

    // Determine physical location of new point to be inserted.
    Point new_pt(0);
    for (MooseIndex(secondary_elem->n_nodes()) n = 0; n < secondary_elem->n_nodes(); ++n)
      new_pt += Moose::fe_lagrange_1D_shape(order, n, xi1) * secondary_elem->point(n);

    // Find the current mortar segment that will have to be split.
    auto & mortar_segment_set = secondary_elems_to_mortar_segments[secondary_elem];
    Elem * current_mortar_segment = nullptr;
    for (const auto & mortar_segment_candidate : mortar_segment_set)
    {
      MortarSegmentInfo * info;
      try
      {
        info = &msm_elem_to_info.at(mortar_segment_candidate);
      }
      catch (std::out_of_range &)
      {
        mooseError("MortarSegmentInfo not found for the mortar segment candidate");
      }
      if (info->xi1_a < xi1 && xi1 < info->xi1_b)
      {
        current_mortar_segment = mortar_segment_candidate;
        break;
      }
    }

    // Make sure we found one.
    if (current_mortar_segment == nullptr)
      mooseError("Unable to find appropriate mortar segment during linear search!");

    Node * new_node = mortar_segment_mesh.add_point(new_pt);

    // Make an Elem on the left
    Elem * new_elem_left;
    if (order == SECOND)
      new_elem_left = mortar_segment_mesh.add_elem(new Edge3);
    else
      new_elem_left = mortar_segment_mesh.add_elem(new Edge2);
    new_elem_left->processor_id() = current_mortar_segment->processor_id();
    new_elem_left->set_interior_parent(current_mortar_segment->interior_parent());
    new_elem_left->set_parent(current_mortar_segment->parent());
    new_elem_left->set_node(0) = current_mortar_segment->node_ptr(0);
    new_elem_left->set_node(1) = new_node;

    // Now for quadratic elements, we have to determine where the interior node
    // lies. We can imagine a new parameterizing coordinate; let's call it
    // eta. Then for the new left element, when eta = -1 then xi = -1, and when
    // eta = 1 then xi = xi1. We want to put the interior node where eta = 0,
    // which will correspond to the location of the point on the pre-split
    // element where xi = (xi1 - 1) / 2
    if (order == SECOND)
    {
      Point left_interior_point(0);
      Real left_interior_xi = (xi1 - 1.) / 2.;
      for (MooseIndex(current_mortar_segment->n_nodes()) n = 0;
           n < current_mortar_segment->n_nodes();
           ++n)
        left_interior_point += Moose::fe_lagrange_1D_shape(order, n, left_interior_xi) *
                               current_mortar_segment->point(n);
      new_elem_left->set_node(2) = mortar_segment_mesh.add_point(left_interior_point);
    }

    // Make an Elem on the right
    Elem * new_elem_right;
    if (order == SECOND)
      new_elem_right = mortar_segment_mesh.add_elem(new Edge3);
    else
      new_elem_right = mortar_segment_mesh.add_elem(new Edge2);
    new_elem_right->processor_id() = current_mortar_segment->processor_id();
    new_elem_right->set_interior_parent(current_mortar_segment->interior_parent());
    new_elem_right->set_parent(current_mortar_segment->parent());
    new_elem_right->set_node(0) = new_node;
    new_elem_right->set_node(1) = current_mortar_segment->node_ptr(1);

    // Now for quadratic elements, we have to determine where the interior node
    // lies. We can imagine a new parameterizing coordinate; let's call it
    // eta. Then for the new right element, when eta = -1 then xi = xi1, and when
    // eta = 1 then xi = 1. We want to put the interior node where eta = 0,
    // which will correspond to the location of the point on the pre-split
    // element where xi = (xi1 + 1) / 2
    if (order == SECOND)
    {
      Point right_interior_point(0);
      Real right_interior_xi = (xi1 + 1.) / 2.;
      for (MooseIndex(current_mortar_segment->n_nodes()) n = 0;
           n < current_mortar_segment->n_nodes();
           ++n)
        right_interior_point += Moose::fe_lagrange_1D_shape(order, n, right_interior_xi) *
                                current_mortar_segment->point(n);
      new_elem_right->set_node(2) = mortar_segment_mesh.add_point(right_interior_point);
    }

    // Reconstruct the nodal normal at xi1. This will help us
    // determine the orientation of the primary elems relative to the
    // new mortar segments.
    Point dxyz_dxi(0);
    for (MooseIndex(secondary_elem->n_nodes()) n = 0; n < secondary_elem->n_nodes(); ++n)
      dxyz_dxi += Moose::fe_lagrange_1D_shape_deriv(order, n, xi1) * secondary_elem->point(n);
    auto normal = Point(dxyz_dxi(1), -dxyz_dxi(0), 0).unit();
    if (_periodic)
      normal *= -1;

    // Get the set of primary_node neighbors.
    if (this->nodes_to_primary_elem_map.find(primary_node->id()) ==
        this->nodes_to_primary_elem_map.end())
      mooseError("We should already have built this primary node to elem pair!");
    const std::vector<const Elem *> & primary_node_neighbors =
        this->nodes_to_primary_elem_map[primary_node->id()];

    // Sanity check
    if (primary_node_neighbors.size() == 0 || primary_node_neighbors.size() > 2)
      mooseError("We must have either 1 or 2 primary side nodal neighbors, but we had ",
                 primary_node_neighbors.size());

    // Primary Elem pointers which we will eventually assign to the
    // mortar segments being created.  We start by assuming
    // primary_node_neighbor[0] is on the "left" and
    // primary_node_neighbor[1]/"nothing" is on the "right" and then
    // swap them if that's not the case.
    const Elem * left_primary_elem = primary_node_neighbors[0];
    const Elem * right_primary_elem =
        (primary_node_neighbors.size() == 2) ? primary_node_neighbors[1] : nullptr;

    Real left_xi2 = MortarSegmentInfo::invalid_xi, right_xi2 = MortarSegmentInfo::invalid_xi;

    // Storage for z-component of cross products for determining
    // orientation.
    std::array<Real, 2> secondary_node_cps, primary_node_cps;

    // Store z-component of left and right secondary node cross products with the nodal normal.
    for (unsigned int nid = 0; nid < 2; ++nid)
      secondary_node_cps[nid] = normal.cross(secondary_elem->point(nid) - new_pt)(2);

    for (MooseIndex(primary_node_neighbors) mnn = 0; mnn < primary_node_neighbors.size(); ++mnn)
    {
      const Elem * primary_neigh = primary_node_neighbors[mnn];
      Point opposite = (primary_neigh->node_ptr(0) == primary_node) ? primary_neigh->point(1)
                                                                    : primary_neigh->point(0);
      Point cp = normal.cross(opposite - new_pt);
      primary_node_cps[mnn] = cp(2);
    }

    // We will verify that only 1 orientation is actually valid.
    bool orientation1_valid = false, orientation2_valid = false;

    if (primary_node_neighbors.size() == 2)
    {
      // 2 primary neighbor case
      orientation1_valid = (secondary_node_cps[0] * primary_node_cps[0] > 0.) &&
                           (secondary_node_cps[1] * primary_node_cps[1] > 0.);

      orientation2_valid = (secondary_node_cps[0] * primary_node_cps[1] > 0.) &&
                           (secondary_node_cps[1] * primary_node_cps[0] > 0.);
    }
    else
    {
      // 1 primary neighbor case
      orientation1_valid = (secondary_node_cps[0] * primary_node_cps[0] > 0.);
      orientation2_valid = (secondary_node_cps[1] * primary_node_cps[0] > 0.);
    }

    // Verify that both orientations are not simultaneously valid/invalid. If they are not, then we
    // are going to throw an exception instead of erroring out since we can easily reach this point
    // if we have one bad linear solve. It's better in general to catch the error and then try a
    // smaller time-step
    if (orientation1_valid && orientation2_valid)
      throw MooseException(
          "AutomaticMortarGeneration: Both orientations cannot simultaneously be valid.");
    if (!orientation1_valid && !orientation2_valid)
      throw MooseException(
          "AutomaticMortarGeneration: Both orientations cannot simultaneously be invalid.");

    // If orientation 2 was valid, swap the left and right primaries.
    if (orientation2_valid)
      std::swap(left_primary_elem, right_primary_elem);

    // Now that we know left_primary_elem and right_primary_elem, we can determine left_xi2 and
    // right_xi2.
    if (left_primary_elem)
      left_xi2 = (primary_node == left_primary_elem->node_ptr(0)) ? -1 : +1;
    if (right_primary_elem)
      right_xi2 = (primary_node == right_primary_elem->node_ptr(0)) ? -1 : +1;

    // Grab the MortarSegmentInfo object associated with this
    // segment. We can use "at()" here since we want this to fail if
    // current_mortar_segment is not found... Since we're going to
    // erase this entry from the map momentarily, we make an actual
    // copy rather than grabbing a reference.
    auto msm_it = msm_elem_to_info.find(current_mortar_segment);
    if (msm_it == msm_elem_to_info.end())
      mooseError("MortarSegmentInfo not found for current_mortar_segment.");
    MortarSegmentInfo current_msinfo = msm_it->second;

    // Create new MortarSegmentInfo objects for new_elem_left and new_elem_right.
    MortarSegmentInfo new_msinfo_left, new_msinfo_right;

    // The new MortarSegmentInfo info objects inherit their "outer"
    // information from current_msinfo and the rest is determined by
    // the Node being inserted.
    new_msinfo_left.xi1_a = current_msinfo.xi1_a;
    new_msinfo_left.xi2_a = current_msinfo.xi2_a;
    new_msinfo_left.secondary_elem = secondary_elem;
    new_msinfo_left.xi1_b = xi1;
    new_msinfo_left.xi2_b = left_xi2;
    new_msinfo_left.primary_elem = left_primary_elem;

    new_msinfo_right.xi1_b = current_msinfo.xi1_b;
    new_msinfo_right.xi2_b = current_msinfo.xi2_b;
    new_msinfo_right.secondary_elem = secondary_elem;
    new_msinfo_right.xi1_a = xi1;
    new_msinfo_right.xi2_a = right_xi2;
    new_msinfo_right.primary_elem = right_primary_elem;

    // Erase the MortarSegmentInfo object for current_mortar_segment from the map.
    msm_elem_to_info.erase(msm_it);

    // Add new_msinfo_left and new_msinfo_right objects to the map.
    msm_elem_to_info.insert(std::make_pair(new_elem_left, new_msinfo_left));
    msm_elem_to_info.insert(std::make_pair(new_elem_right, new_msinfo_right));

    // current_mortar_segment must be erased from the
    // mortar_segment_set since it has now been split.
    mortar_segment_set.erase(current_mortar_segment);

    // The original mortar segment has been split, so erase it from
    // the mortar segment mesh.
    mortar_segment_mesh.delete_elem(current_mortar_segment);

    // We need to insert new_elem_left and new_elem_right in
    // the mortar_segment_set for this secondary_elem.
    mortar_segment_set.insert(new_elem_left);
    mortar_segment_set.insert(new_elem_right);
  }

  // Set up the the mortar segment neighbor information.
  mortar_segment_mesh.allow_renumbering(true);
  mortar_segment_mesh.skip_partitioning(true);
  mortar_segment_mesh.allow_find_neighbors(false);
  mortar_segment_mesh.prepare_for_use();
  mortar_segment_mesh.allow_find_neighbors(true);

  // (Optionally) Write the mortar segment mesh to file for inspection
  if (_debug)
  {
    ExodusII_IO mortar_segment_mesh_writer(mortar_segment_mesh);
    mortar_segment_mesh_writer.write("mortar_segment_mesh.e");
  }

  // Loop over the msm_elem_to_info object and build a bi-directional
  // multimap from secondary elements to the primary Elems which they are
  // coupled to and vice-versa. This is used in the
  // AugmentSparsityOnInterface functor to determine whether a given
  // secondary Elem is coupled across the mortar interface to a primary
  // element.
  for (const auto & pr : msm_elem_to_info)
  {
    const Elem * secondary_elem = pr.second.secondary_elem;
    const Elem * primary_elem = pr.second.primary_elem;

    // The interior_parent() element should also be coupled to the
    // secondary_elem. The coupling in the other direction (secondary_elem
    // -> secondary_elem->interior_parent()) happens automatically.
    mortar_interface_coupling.insert(
        std::make_pair(secondary_elem->interior_parent()->id(), secondary_elem->id()));

    // Insert both Elems as key and value.
    if (primary_elem)
    {
      mortar_interface_coupling.insert(std::make_pair(secondary_elem->id(), primary_elem->id()));
      mortar_interface_coupling.insert(std::make_pair(primary_elem->id(), secondary_elem->id()));

      // Also insert both interior parents as key and value.
      mortar_interface_coupling.insert(std::make_pair(secondary_elem->interior_parent()->id(),
                                                      primary_elem->interior_parent()->id()));
      mortar_interface_coupling.insert(std::make_pair(primary_elem->interior_parent()->id(),
                                                      secondary_elem->interior_parent()->id()));
    }
  }
}

void
AutomaticMortarGeneration::computeNodalNormals()
{
  // The dimension according to Mesh::mesh_dimension().
  const auto dim = mesh.mesh_dimension();

  // Build FEType objects for the different variables. This order and
  // family isn't that important because we are only using it for
  // geometric information...
  FEType nnx_fe_type(FIRST, LAGRANGE);

  // Build FE objects from those types.
  std::unique_ptr<FEBase> nnx_fe_face(FEBase::build(dim, nnx_fe_type));

  // A nodal lower-dimensional trapezoidal quadrature rule to be used on faces.
  QTrap qface(dim - 1);

  // Tell the FE objects about the quadrature rule.
  nnx_fe_face->attach_quadrature_rule(&qface);

  // Get a reference to the normals from the face FE.
  const std::vector<Point> & face_normals = nnx_fe_face->get_normals();

  // A map from the node id to the attached elemental normals evaluated at the node
  std::map<dof_id_type, std::vector<Point>> node_to_normals_map;

  /// The _periodic flag tells us whether we want to inward vs outward facing normals
  Real sign = _periodic ? -1 : 1;

  // First loop over lower-dimensional secondary side elements and compute/save the outward normal
  // for each one. We loop over all active elements currently, but this procedure could be
  // parallelized as well.
  for (MeshBase::const_element_iterator el = mesh.active_elements_begin(),
                                        end_el = mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * secondary_elem = *el;

    // If this is not one of the lower-dimensional secondary side elements, go on to the next one.
    if (!this->secondary_boundary_subdomain_ids.count(secondary_elem->subdomain_id()))
      continue;

    // Which side of the parent are we? We need to know this to know
    // which side to reinit.
    const Elem * interior_parent = secondary_elem->interior_parent();
    mooseAssert(interior_parent,
                "No interior parent exists for element "
                    << secondary_elem->id()
                    << ". There may be a problem with your sideset set-up.");

    // Look up which side of the interior parent secondary_elem is.
    auto s = interior_parent->which_side_am_i(secondary_elem);

    // Reinit the face FE object on side s.
    nnx_fe_face->reinit(interior_parent, s);

    // We loop over n_vertices not n_nodes because we're not interested in
    // computing normals at interior nodes
    for (MooseIndex(secondary_elem->n_vertices()) n = 0; n < secondary_elem->n_vertices(); ++n)
    {
      auto & normals_vec = node_to_normals_map[secondary_elem->node_id(n)];
      normals_vec.push_back(sign * face_normals[n]);
    }
  }

  // Note that contrary to the Bin Yang dissertation, we are not weighting by the face element
  // lengths/volumes. It's not clear to me that this type of weighting is a good algorithm for cases
  // where the face can be curved
  for (const auto & pr : node_to_normals_map)
  {
    const auto & node_id = pr.first;
    const auto & normals_vec = pr.second;

    Point nodal_normal;
    for (const auto & normals : normals_vec)
      nodal_normal += normals;

    secondary_node_to_nodal_normal[mesh.node_ptr(node_id)] = nodal_normal.unit();
  }
}

// Project secondary nodes onto their corresponding primary elements for each primary/secondary
// pair.
void
AutomaticMortarGeneration::projectSecondaryNodes()
{
  // For each primary/secondary boundary id pair, call the
  // project_secondary_nodes_single_pair() helper function.
  for (const auto & pr : primary_secondary_subdomain_id_pairs)
    projectSecondaryNodesSinglePair(pr.first, pr.second);
}

void
AutomaticMortarGeneration::projectSecondaryNodesSinglePair(
    subdomain_id_type lower_dimensional_primary_subdomain_id,
    subdomain_id_type lower_dimensional_secondary_subdomain_id)
{
  // Build the "subdomain" adaptor based KD Tree.
  NanoflannMeshSubdomainAdaptor<3> mesh_adaptor(mesh, lower_dimensional_primary_subdomain_id);
  subdomain_kd_tree_t kd_tree(
      3, mesh_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(/*max leaf=*/10));

  // Construct the KD tree.
  kd_tree.buildIndex();

  for (MeshBase::const_element_iterator el = mesh.active_elements_begin(),
                                        end_el = mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * secondary_side_elem = *el;

    // If this Elem is not in the current secondary subodmain, go on to the next one.
    if (secondary_side_elem->subdomain_id() != lower_dimensional_secondary_subdomain_id)
      continue;

    // For each node on the lower-dimensional element, find the nearest
    // node on the primary side using the KDTree, then
    // search in nearby elements for where it projects
    // along the nodal normal direction.
    for (MooseIndex(secondary_side_elem->n_vertices()) n = 0; n < secondary_side_elem->n_vertices();
         ++n)
    {
      const Node * secondary_node = secondary_side_elem->node_ptr(n);

      // Get the nodal neighbors for secondary_node, so we can check whether we've
      // already successfully projected it.
      const std::vector<const Elem *> & secondary_node_neighbors =
          this->nodes_to_secondary_elem_map.at(secondary_node->id());

      // Check whether we've already mapped this secondary node
      // successfully for all of its nodal neighbors.
      bool is_mapped = true;
      for (MooseIndex(secondary_node_neighbors) snn = 0; snn < secondary_node_neighbors.size();
           ++snn)
      {
        auto secondary_key = std::make_pair(secondary_node, secondary_node_neighbors[snn]);
        if (!secondary_node_and_elem_to_xi2_primary_elem.count(secondary_key))
        {
          is_mapped = false;
          break;
        }
      }

      // Go to the next node if this one has already been mapped.
      if (is_mapped)
        continue;

      // Look up the new nodal normal value in the local storage, error if not found.
      Point nodal_normal = this->secondary_node_to_nodal_normal.at(secondary_node);

      // Data structure for performing Nanoflann searches.
      std::array<Real, 3> query_pt = {
          {(*secondary_node)(0), (*secondary_node)(1), (*secondary_node)(2)}};

      // The number of results we want to get.  We'll look for a
      // "few" nearest nodes, hopefully that is enough to let us
      // figure out which lower-dimensional Elem on the primary
      // side we are across from.
      const std::size_t num_results = 3;

      // Initialize result_set and do the search.
      std::vector<size_t> ret_index(num_results);
      std::vector<Real> out_dist_sqr(num_results);
      nanoflann::KNNResultSet<Real> result_set(num_results);
      result_set.init(&ret_index[0], &out_dist_sqr[0]);
      kd_tree.findNeighbors(result_set, &query_pt[0], nanoflann::SearchParams(10));

      // If this flag gets set in the loop below, we can break out of the outer r-loop as well.
      bool projection_succeeded = false;

      // Once we've rejected a candidate for a given secondary_node,
      // there's no reason to check it again.
      std::set<const Elem *> rejected_primary_elem_candidates;

      // Loop over the closest nodes, check whether
      // the secondary node successfully projects into
      // either of the closest neighbors, stop when
      // the projection succeeds.
      for (MooseIndex(result_set) r = 0; r < result_set.size(); ++r)
      {
        // Verify that the squared distance we compute is the same as nanoflann's
        if (std::abs((mesh.point(ret_index[r]) - *secondary_node).norm_sq() - out_dist_sqr[r]) >
            TOLERANCE)
          mooseError("Lower-dimensional element squared distance verification failed.");

        // Get a reference to the vector of lower dimensional elements from the
        // nodes_to_primary_elem_map.
        std::vector<const Elem *> & primary_elem_candidates =
            this->nodes_to_primary_elem_map.at(static_cast<dof_id_type>(ret_index[r]));

        // Search the Elems connected to this node on the primary mesh side.
        for (MooseIndex(primary_elem_candidates) e = 0; e < primary_elem_candidates.size(); ++e)
        {
          const Elem * primary_elem_candidate = primary_elem_candidates[e];

          // If we've already rejected this candidate, we don't need to check it again.
          if (rejected_primary_elem_candidates.count(primary_elem_candidate))
            continue;

          // Now generically solve for xi2
          auto && order = primary_elem_candidate->default_order();
          DualNumber<Real> xi2_dn{0, 1};
          unsigned int current_iterate = 0, max_iterates = 10;

          // Newton loop
          do
          {
            VectorValue<DualNumber<Real>> x2(0);
            for (MooseIndex(primary_elem_candidate->n_nodes()) n = 0;
                 n < primary_elem_candidate->n_nodes();
                 ++n)
              x2 +=
                  Moose::fe_lagrange_1D_shape(order, n, xi2_dn) * primary_elem_candidate->point(n);
            auto u = x2 - (*secondary_node);
            auto F = u(0) * nodal_normal(1) - u(1) * nodal_normal(0);

            if (std::abs(F) < TOLERANCE)
              break;

            Real dxi2 = -F.value() / F.derivatives();

            xi2_dn += dxi2;
          } while (++current_iterate < max_iterates);

          Real xi2 = xi2_dn.value();

          // Check whether the projection worked.
          if (std::abs(xi2) <= 1. + TOLERANCE)
          {
            // If xi2 == +1 or -1 then this secondary node mapped directly to a node on the primary
            // surface. This isn't as unlikely as you might think, it will happen if the meshes
            // on the interface start off being perfectly aligned. In this situation, we need to
            // associate the secondary node with two different elements (and two corresponding
            // xi^(2) values.
            if (std::abs(std::abs(xi2) - 1.) < TOLERANCE)
            {
              const Node * primary_node = (xi2 < 0) ? primary_elem_candidate->node_ptr(0)
                                                    : primary_elem_candidate->node_ptr(1);

              const std::vector<const Elem *> & primary_node_neighbors =
                  this->nodes_to_primary_elem_map.at(primary_node->id());

              std::vector<bool> primary_elems_mapped(primary_node_neighbors.size(), false);

              // Add entries to secondary_node_and_elem_to_xi2_primary_elem container.
              //
              // First, determine "on left" vs. "on right" orientation of the nodal neighbors.
              // There can be a max of 2 nodal neighbors, and we want to make sure that the
              // secondary nodal neighbor on the "left" is associated with the primary nodal
              // neighbor on the "left" and similarly for the "right".
              std::vector<Real> secondary_node_neighbor_cps(2), primary_node_neighbor_cps(2);

              // Figure out which secondary side neighbor is on the "left" and which is on the
              // "right".
              for (MooseIndex(secondary_node_neighbors) nn = 0;
                   nn < secondary_node_neighbors.size();
                   ++nn)
              {
                const Elem * secondary_neigh = secondary_node_neighbors[nn];
                Point opposite = (secondary_neigh->node_ptr(0) == secondary_node)
                                     ? secondary_neigh->point(1)
                                     : secondary_neigh->point(0);
                Point cp = nodal_normal.cross(opposite - *secondary_node);
                secondary_node_neighbor_cps[nn] = cp(2);
              }

              // Figure out which primary side neighbor is on the "left" and which is on the
              // "right".
              for (MooseIndex(primary_node_neighbors) nn = 0; nn < primary_node_neighbors.size();
                   ++nn)
              {
                const Elem * primary_neigh = primary_node_neighbors[nn];
                Point opposite = (primary_neigh->node_ptr(0) == primary_node)
                                     ? primary_neigh->point(1)
                                     : primary_neigh->point(0);
                Point cp = nodal_normal.cross(opposite - *secondary_node);
                primary_node_neighbor_cps[nn] = cp(2);
              }

              // Associate secondary/primary elems on matching sides.
              bool found_match = false;
              for (MooseIndex(secondary_node_neighbors) snn = 0;
                   snn < secondary_node_neighbors.size();
                   ++snn)
                for (MooseIndex(primary_node_neighbors) mnn = 0;
                     mnn < primary_node_neighbors.size();
                     ++mnn)
                  if (secondary_node_neighbor_cps[snn] * primary_node_neighbor_cps[mnn] > 0)
                  {
                    found_match = true;
                    primary_elems_mapped[mnn] = true;

                    // Figure out xi^(2) value by looking at which node primary_node is
                    // of the current primary node neighbor.
                    Real xi2 = (primary_node == primary_node_neighbors[mnn]->node_ptr(0)) ? -1 : +1;
                    auto secondary_key =
                        std::make_pair(secondary_node, secondary_node_neighbors[snn]);
                    auto primary_val = std::make_pair(xi2, primary_node_neighbors[mnn]);
                    secondary_node_and_elem_to_xi2_primary_elem.insert(
                        std::make_pair(secondary_key, primary_val));

                    // Also map in the other direction.
                    Real xi1 =
                        (secondary_node == secondary_node_neighbors[snn]->node_ptr(0)) ? -1 : +1;
                    auto primary_key = std::make_tuple(
                        primary_node->id(), primary_node, primary_node_neighbors[mnn]);
                    auto secondary_val = std::make_pair(xi1, secondary_node_neighbors[snn]);
                    primary_node_and_elem_to_xi1_secondary_elem.insert(
                        std::make_pair(primary_key, secondary_val));
                  }

              // Sanity check
              if (!found_match)
                mooseError("Could not associate primary/secondary neighbors on either side of "
                           "secondary_node.");

              // We need to handle the case where we've exactly projected a secondary node onto a
              // primary node, but our secondary node is at one of the secondary face endpoints and
              // our primary node is not.
              if (secondary_node_neighbors.size() == 1 && primary_node_neighbors.size() == 2)
                for (auto it = primary_elems_mapped.begin(); it != primary_elems_mapped.end(); ++it)
                  if (*it == false)
                  {
                    auto index = std::distance(primary_elems_mapped.begin(), it);
                    primary_node_and_elem_to_xi1_secondary_elem.insert(std::make_pair(
                        std::make_tuple(
                            primary_node->id(), primary_node, primary_node_neighbors[index]),
                        std::make_pair(1, nullptr)));
                  }
            }
            else // Point falls somewhere in the middle of the Elem.
            {
              // Add two entries to secondary_node_and_elem_to_xi2_primary_elem.
              for (MooseIndex(secondary_node_neighbors) nn = 0;
                   nn < secondary_node_neighbors.size();
                   ++nn)
              {
                const Elem * neigh = secondary_node_neighbors[nn];
                for (MooseIndex(neigh->n_vertices()) nid = 0; nid < neigh->n_vertices(); ++nid)
                {
                  const Node * neigh_node = neigh->node_ptr(nid);
                  if (secondary_node == neigh_node)
                  {
                    auto key = std::make_pair(neigh_node, neigh);
                    auto val = std::make_pair(xi2, primary_elem_candidate);
                    secondary_node_and_elem_to_xi2_primary_elem.insert(std::make_pair(key, val));
                  }
                }
              }
            }

            projection_succeeded = true;
            break; // out of e-loop
          }
          else
          {
            // The current secondary_node is not in this Elem, so keep track of the rejects.
            rejected_primary_elem_candidates.insert(primary_elem_candidate);
          }
        }

        if (projection_succeeded)
          break; // out of r-loop
      }          // r-loop

      if (!projection_succeeded && _debug)
      {
        _console << "Failed to find primary Elem into which secondary node "
                 << static_cast<const Point &>(*secondary_node) << " was projected." << std::endl
                 << std::endl;
        ;
      }
    } // loop over side nodes
  }   // end loop over lower-dimensional elements
}

// Inverse map primary nodes onto their corresponding secondary elements for each primary/secondary
// pair.
void
AutomaticMortarGeneration::projectPrimaryNodes()
{
  // For each primary/secondary boundary id pair, call the
  // project_primary_nodes_single_pair() helper function.
  for (const auto & pr : primary_secondary_subdomain_id_pairs)
    projectPrimaryNodesSinglePair(pr.first, pr.second);
}

void
AutomaticMortarGeneration::projectPrimaryNodesSinglePair(
    subdomain_id_type lower_dimensional_primary_subdomain_id,
    subdomain_id_type lower_dimensional_secondary_subdomain_id)
{
  // Build a Nanoflann object on the lower-dimensional secondary elements of the Mesh.
  NanoflannMeshSubdomainAdaptor<3> mesh_adaptor(mesh, lower_dimensional_secondary_subdomain_id);
  subdomain_kd_tree_t kd_tree(
      3, mesh_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(/*max leaf=*/10));

  // Construct the KD tree for lower-dimensional elements in the volume mesh.
  kd_tree.buildIndex();

  for (const auto & primary_side_elem : mesh.active_element_ptr_range())
  {
    // If this is not one of the lower-dimensional primary side elements, go on to the next one.
    if (primary_side_elem->subdomain_id() != lower_dimensional_primary_subdomain_id)
      continue;

    // For each node on this side, find the nearest node on the secondary side using the KDTree,
    // then search in nearby elements for where it projects along the nodal normal direction.
    for (MooseIndex(primary_side_elem->n_vertices()) n = 0; n < primary_side_elem->n_vertices();
         ++n)
    {
      // Get a pointer to this node.
      const Node * primary_node = primary_side_elem->node_ptr(n);

      // Get the nodal neighbors connected to this primary node.
      const std::vector<const Elem *> & primary_node_neighbors =
          this->nodes_to_primary_elem_map.at(primary_node->id());

      // Check whether we have already successfully inverse mapped this primary node and skip if
      // so.
      auto primary_key =
          std::make_tuple(primary_node->id(), primary_node, primary_node_neighbors[0]);
      if (primary_node_and_elem_to_xi1_secondary_elem.count(primary_key))
        continue;

      // Data structure for performing Nanoflann searches.
      Real query_pt[3] = {(*primary_node)(0), (*primary_node)(1), (*primary_node)(2)};

      // The number of results we want to get.  We'll look for a
      // "few" nearest nodes, hopefully that is enough to let us
      // figure out which lower-dimensional Elem on the secondary side
      // we are across from.
      const size_t num_results = 3;

      // Initialize result_set and do the search.
      std::vector<size_t> ret_index(num_results);
      std::vector<Real> out_dist_sqr(num_results);
      nanoflann::KNNResultSet<Real> result_set(num_results);
      result_set.init(&ret_index[0], &out_dist_sqr[0]);
      kd_tree.findNeighbors(result_set, &query_pt[0], nanoflann::SearchParams(10));

      // If this flag gets set in the loop below, we can break out of the outer r-loop as well.
      bool projection_succeeded = false;

      // Once we've rejected a candidate for a given
      // primary_node, there's no reason to check it
      // again.
      std::set<const Elem *> rejected_secondary_elem_candidates;

      // Loop over the closest nodes, check whether the secondary node successfully projects into
      // either of the closest neighbors, stop when the projection succeeds.
      for (MooseIndex(result_set) r = 0; r < result_set.size(); ++r)
      {
        // Verify that the squared distance we compute is the same as nanoflann's
        if (std::abs((mesh.point(ret_index[r]) - *primary_node).norm_sq() - out_dist_sqr[r]) >
            TOLERANCE)
          mooseError("Lower-dimensional element squared distance verification failed.");

        // Get a reference to the vector of lower dimensional elements from the
        // nodes_to_secondary_elem_map.
        const std::vector<const Elem *> & secondary_elem_candidates =
            this->nodes_to_secondary_elem_map.at(static_cast<dof_id_type>(ret_index[r]));

        // Print the Elems connected to this node on the secondary mesh side.
        for (MooseIndex(secondary_elem_candidates) e = 0; e < secondary_elem_candidates.size(); ++e)
        {
          const Elem * secondary_elem_candidate = secondary_elem_candidates[e];

          // If we've already rejected this candidate, we don't need to check it again.
          if (rejected_secondary_elem_candidates.count(secondary_elem_candidate))
            continue;

          // Use equation 2.4.6 from Bin Yang's dissertation to try and solve for
          // the position on the secondary element where this primary came from.  This
          // requires a Newton iteration in general.
          DualNumber<Real> xi1_dn{0, 1}; // initial guess
          auto && order = secondary_elem_candidate->default_order();
          unsigned int current_iterate = 0, max_iterates = 10;

          // Newton iteration loop - this to converge in 1 iteration when it
          // succeeds, and possibly two iterations when it converges to a
          // xi outside the reference element. I don't know any reason why it should
          // only take 1 iteration -- the Jacobian is not constant in general...
          do
          {
            VectorValue<DualNumber<Real>> x1(0);
            VectorValue<DualNumber<Real>> dx1_dxi(0);
            for (MooseIndex(secondary_elem_candidate->n_nodes()) n = 0;
                 n < secondary_elem_candidate->n_nodes();
                 ++n)
            {
              x1 += Moose::fe_lagrange_1D_shape(order, n, xi1_dn) *
                    secondary_elem_candidate->point(n);
              dx1_dxi += Moose::fe_lagrange_1D_shape_deriv(order, n, xi1_dn) *
                         secondary_elem_candidate->point(n);
            }

            // We're assuming our mesh is in the xy plane here
            auto normals = VectorValue<DualNumber<Real>>(dx1_dxi(1), -dx1_dxi(0), 0).unit();
            if (_periodic)
              normals *= -1;

            auto u = x1 - (*primary_node);

            auto F = u(0) * normals(1) - u(1) * normals(0);

            if (std::abs(F) < TOLERANCE)
              break;

            Real dxi1 = -F.value() / F.derivatives();

            xi1_dn += dxi1;
          } while (++current_iterate < max_iterates);

          Real xi1 = xi1_dn.value();

          // Check for convergence to a valid solution...
          if (std::abs(xi1) <= 1. + TOLERANCE)
          {
            if (std::abs(std::abs(xi1) - 1.) < TOLERANCE)
            {
              // Special case: xi1=+/-1.
              // We shouldn't get here, because this primary node should already
              // have been mapped during the project_secondary_nodes() routine.
              throw MooseException("We should never get here, aligned primary nodes should already "
                                   "have been mapped.");
            }
            else // somewhere in the middle of the Elem
            {
              // Add entry to primary_node_and_elem_to_xi1_secondary_elem
              //
              // Note: we originally duplicated the map values for the keys (node, left_neighbor)
              // and (node, right_neighbor) but I don't think that should be necessary. Instead we
              // just do it for neighbor 0, but really maybe we don't even need to do that since
              // we can always look up the neighbors later given the Node... keeping it like this
              // helps to maintain the "symmetry" of the two containers.
              const Elem * neigh = primary_node_neighbors[0];
              for (MooseIndex(neigh->n_vertices()) nid = 0; nid < neigh->n_vertices(); ++nid)
              {
                const Node * neigh_node = neigh->node_ptr(nid);
                if (primary_node == neigh_node)
                {
                  auto key = std::make_tuple(neigh_node->id(), neigh_node, neigh);
                  auto val = std::make_pair(xi1, secondary_elem_candidate);
                  primary_node_and_elem_to_xi1_secondary_elem.insert(std::make_pair(key, val));
                }
              }
            }

            projection_succeeded = true;
            break; // out of e-loop
          }
          else
          {
            // The current primary_point is not in this Elem, so keep track of the rejects.
            rejected_secondary_elem_candidates.insert(secondary_elem_candidate);
          }
        } // end e-loop over candidate elems

        if (projection_succeeded)
          break; // out of r-loop
      }          // r-loop

      if (!projection_succeeded && _debug)
      {
        _console << "Failed to find point from which primary node "
                 << static_cast<const Point &>(*primary_node) << " was projected." << std::endl
                 << std::endl;
      }
    } // loop over side nodes
  }   // end loop over elements for finding where primary points would have projected from.
}

void
AutomaticMortarGeneration::writeNodalNormalsToFile()
{
  // Must call compute_nodal_normals() first!
  if (secondary_node_to_nodal_normal.empty())
    mooseError("No entries found in the secondary node -> nodal normal map.");

  // Note: I seem to remember an issue with creating a second
  // EquationSystems with the same Mesh, but this code seems to work
  // OK currently.
  EquationSystems nodal_normals_es(this->mesh);
  ExplicitSystem & nodal_normals_system =
      nodal_normals_es.add_system<ExplicitSystem>("nodal_normals");
  auto nnx_var_num = nodal_normals_system.add_variable("nodal_normal_x", FEType(FIRST, LAGRANGE)),
       nny_var_num = nodal_normals_system.add_variable("nodal_normal_y", FEType(FIRST, LAGRANGE));
  nodal_normals_es.init();

  const DofMap & dof_map = nodal_normals_system.get_dof_map();
  std::vector<dof_id_type> dof_indices_nnx, dof_indices_nny;

  for (MeshBase::const_element_iterator el = mesh.elements_begin(), end_el = mesh.elements_end();
       el != end_el;
       ++el)
  {
    const Elem * elem = *el;

    // Get the nodal dofs for this Elem.
    dof_map.dof_indices(elem, dof_indices_nnx, nnx_var_num);
    dof_map.dof_indices(elem, dof_indices_nny, nny_var_num);

    // For each node of the Elem, if it is in the secondary_node_to_nodal_normal
    // container, set the corresponding nodal normal dof values.
    for (MooseIndex(elem->n_vertices()) n = 0; n < elem->n_vertices(); ++n)
    {
      auto it = this->secondary_node_to_nodal_normal.find(elem->node_ptr(n));
      if (it != this->secondary_node_to_nodal_normal.end())
      {
        nodal_normals_system.solution->set(dof_indices_nnx[n], it->second(0));
        nodal_normals_system.solution->set(dof_indices_nny[n], it->second(1));
      }
    } // end loop over nodes
  }   // end loop over elems

  // Finish assembly.
  nodal_normals_system.solution->close();

  // Write the nodal normals to file
  ExodusII_IO(this->mesh).write_equation_systems("nodal_normals_only.e", nodal_normals_es);
}
