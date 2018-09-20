// App headers
#include "AutomaticMortarGeneration.h"
#include "MortarSegmentInfo.h"
#include "NanoflannMeshAdaptor.h"

// libMesh headers
#include "libmesh/mesh_tools.h"
#include "libmesh/getpot.h"
#include "libmesh/explicit_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/dof_map.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/quadrature_gauss.h"

using namespace libMesh;

const std::string AutomaticMortarGeneration::system_name = "Nodal Normals";

AutomaticMortarGeneration::AutomaticMortarGeneration(
    MeshBase & mesh_in,
    const std::pair<BoundaryID, BoundaryID> & boundary_key,
    const std::pair<SubdomainID, SubdomainID> & subdomain_key)
  : mesh(mesh_in), mortar_segment_mesh(mesh_in.comm()), h_max(0.)
{
  master_slave_boundary_id_pairs.push_back(boundary_key);
  master_requested_boundary_ids.insert(boundary_key.first);
  slave_requested_boundary_ids.insert(boundary_key.second);
  master_boundary_subdomain_ids.insert(subdomain_key.first);
  slave_boundary_subdomain_ids.insert(subdomain_key.second);
}

void
AutomaticMortarGeneration::build_node_to_elem_maps()
{
  if (slave_requested_boundary_ids.empty() || master_requested_boundary_ids.empty())
    libmesh_error_msg(
        "Must specify slave and master boundary ids before building node-to-elem maps.");

  // Construct nodes_to_slave_elem_map
  for (MeshBase::const_element_iterator el = mesh.elements_begin(), end_el = mesh.elements_end();
       el != end_el;
       ++el)
  {
    const Elem * slave_elem = *el;

    // If this is not one of the lower-dimensional slave side elements, go on to the next one.
    if (!this->slave_boundary_subdomain_ids.count(slave_elem->subdomain_id()))
      continue;

    for (unsigned int n = 0; n < slave_elem->n_nodes(); ++n)
    {
      std::vector<const Elem *> & vec = nodes_to_slave_elem_map[slave_elem->node_id(n)];
      vec.push_back(slave_elem);
    }
  }

  // Construct nodes_to_master_elem_map
  for (MeshBase::const_element_iterator el = mesh.elements_begin(), end_el = mesh.elements_end();
       el != end_el;
       ++el)
  {
    const Elem * master_elem = *el;

    // If this is not one of the lower-dimensional master side elements, go on to the next one.
    if (!this->master_boundary_subdomain_ids.count(master_elem->subdomain_id()))
      continue;

    for (unsigned int n = 0; n < master_elem->n_nodes(); ++n)
    {
      std::vector<const Elem *> & vec = nodes_to_master_elem_map[master_elem->node_id(n)];
      vec.push_back(master_elem);
    }
  }

  // Debugging: print the contents of the map
  // for (const auto & pr : nodes_to_slave_elem_map)
  //   {
  //     libMesh::out << "Elems connected to node " << pr.first << ": ";
  //     for (const auto & elem : pr.second)
  //       libMesh::out << elem->id() << " ";
  //     libMesh::out << std::endl;
  //   }

  // Debugging: print the contents of the map
  // for (const auto & pr : nodes_to_master_elem_map)
  //   {
  //     libMesh::out << "Elems connected to node " << pr.first << ": ";
  //     for (const auto & elem : pr.second)
  //       libMesh::out << elem->id() << " ";
  //     libMesh::out << std::endl;
  //   }
}

void
AutomaticMortarGeneration::build_mortar_segment_mesh()
{
  // We maintain a mapping from lower-dimensional slave elements in
  // the original mesh to (sets of) elements in mortar_segment_mesh.
  // This allows us to quickly determine which elements need to be
  // split.
  std::map<const Elem *, std::set<Elem *>> slave_elems_to_mortar_segments;

  // 1.) Add all lower-dimensional slave side elements as the "initial" mortar segments.
  for (MeshBase::const_element_iterator el = mesh.active_elements_begin(),
                                        end_el = mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * slave_elem = *el;

    // If this is not one of the lower-dimensional slave side elements, go on to the next one.
    if (!this->slave_boundary_subdomain_ids.count(slave_elem->subdomain_id()))
      continue;

    Node * new_node0 = mortar_segment_mesh.add_point(slave_elem->point(0), slave_elem->node_id(0));
    Node * new_node1 = mortar_segment_mesh.add_point(slave_elem->point(1), slave_elem->node_id(1));

    Elem * new_elem = mortar_segment_mesh.add_elem(new Edge2);

    new_elem->set_node(0) = new_node0;
    new_elem->set_node(1) = new_node1;

    // The xi^(1) values for this mortar segment are initially -1 and 1.
    MortarSegmentInfo msinfo;
    msinfo.xi1_a = -1;
    msinfo.xi1_b = +1;
    msinfo.slave_elem = slave_elem;

    auto new_container_it0 = slave_node_and_elem_to_xi2_master_elem.find(
             std::make_pair(slave_elem->node_ptr(0), slave_elem)),
         new_container_it1 = slave_node_and_elem_to_xi2_master_elem.find(
             std::make_pair(slave_elem->node_ptr(1), slave_elem));

    bool new_container_node0_found =
             (new_container_it0 != slave_node_and_elem_to_xi2_master_elem.end()),
         new_container_node1_found =
             (new_container_it1 != slave_node_and_elem_to_xi2_master_elem.end());

    const Elem * node0_master_candidate = libmesh_nullptr;
    const Elem * node1_master_candidate = libmesh_nullptr;

    if (new_container_node0_found)
    {
      const auto & xi2_master_elem_pair = new_container_it0->second;
      msinfo.xi2_a = xi2_master_elem_pair.first;
      node0_master_candidate = xi2_master_elem_pair.second;
    }

    if (new_container_node1_found)
    {
      const auto & xi2_master_elem_pair = new_container_it1->second;
      msinfo.xi2_b = xi2_master_elem_pair.first;
      node1_master_candidate = xi2_master_elem_pair.second;
    }

    // If both node0 and node1 agree on the master element they are
    // projected into, then this mortar segment fits entirely within
    // a single master element, and we can go ahead and set the
    // msinfo.master_elem pointer now.
    if (node0_master_candidate == node1_master_candidate)
      msinfo.master_elem = node0_master_candidate;

    // Associate this MSM elem with the MortarSegmentInfo.
    msm_elem_to_info.insert(std::make_pair(new_elem, msinfo));

    // Maintain the mapping between slave elems and mortar segment elems contained within them.
    // Initially, only the original slave_elem is present.
    slave_elems_to_mortar_segments[slave_elem].insert(new_elem);
  }

  // 2.) Insert new nodes from master side and split mortar segments as necessary.
  for (const auto & pr : master_node_and_elem_to_xi1_slave_elem)
  {
    auto key = pr.first;
    auto val = pr.second;

    const Node * master_node = key.first;
    Real xi1 = val.first;
    const Elem * slave_elem = val.second;

    // If this is an aligned node, we don't need to do anything.
    if (std::abs(std::abs(xi1) - 1.) < TOLERANCE)
      continue;

    // Determine physical location of new point to be inserted.
    Point new_pt = 0.5 * (1 - xi1) * slave_elem->point(0) + 0.5 * (1 + xi1) * slave_elem->point(1);

    // Find the current mortar segment that will have to be split.
    auto & mortar_segment_set = slave_elems_to_mortar_segments[slave_elem];
    Elem * current_mortar_segment = libmesh_nullptr;
    for (const auto & mortar_segment_candidate : mortar_segment_set)
    {
      // Test whether new_pt lies in the element by checking
      // whether the sum of the distances from the endpoints to
      // the new point is approximately equal to the distance
      // between the endpoints.
      Point a = mortar_segment_candidate->point(0), b = mortar_segment_candidate->point(1);
      if (std::abs((a - new_pt).norm() + (b - new_pt).norm() - (b - a).norm()) < TOLERANCE)
      {
        current_mortar_segment = mortar_segment_candidate;
        break;
      }
    }

    // Make sure we found one.
    if (current_mortar_segment == libmesh_nullptr)
      libmesh_error_msg("Unable to find appropriate mortar segment during linear search!");

    Node * new_node = mortar_segment_mesh.add_point(new_pt);

    // Make an Elem on the left
    Elem * new_elem_left = mortar_segment_mesh.add_elem(new Edge2);
    new_elem_left->set_node(0) = current_mortar_segment->node_ptr(0);
    new_elem_left->set_node(1) = new_node;

    // Make an Elem on the right
    Elem * new_elem_right = mortar_segment_mesh.add_elem(new Edge2);
    new_elem_right->set_node(0) = new_node;
    new_elem_right->set_node(1) = current_mortar_segment->node_ptr(1);

    // Reconstruct the nodal normal at xi1. This will help us
    // determine the orientation of the master elems relative to the
    // new mortar segments.
    Point nodal_normal_left = this->slave_node_to_nodal_normal.at(slave_elem->node_ptr(0)),
          nodal_normal_right = this->slave_node_to_nodal_normal.at(slave_elem->node_ptr(1));

    Point nodal_normal = 0.5 * (1 - xi1) * nodal_normal_left + 0.5 * (1 + xi1) * nodal_normal_right;

    // Get the set of master_node neighbors.
    const std::vector<const Elem *> & master_node_neighbors =
        this->nodes_to_master_elem_map.at(master_node->id());

    // Sanity check
    if (master_node_neighbors.size() == 0 || master_node_neighbors.size() > 2)
      libmesh_error_msg("We must have either 1 or 2 master side nodal neighbors, but we had "
                        << master_node_neighbors.size());

    // Master Elem pointers which we will eventually assign to the
    // mortar segments being created.  We start by assuming
    // master_node_neighbor[0] is on the "left" and
    // master_node_neighbor[1]/"nothing" is on the "right" and then
    // swap them if that's not the case.
    const Elem * left_master_elem = master_node_neighbors[0];
    const Elem * right_master_elem =
        (master_node_neighbors.size() == 2) ? master_node_neighbors[1] : libmesh_nullptr;

    Real left_xi2 = MortarSegmentInfo::invalid_xi, right_xi2 = MortarSegmentInfo::invalid_xi;

    // Storage for z-component of cross products for determining
    // orientation.
    std::vector<Real> slave_node_cps(2), master_node_cps(2);

    // Store z-component of left and right slave node cross products with the nodal normal.
    for (unsigned int nid = 0; nid < 2; ++nid)
      slave_node_cps[nid] = nodal_normal.cross(slave_elem->point(nid) - new_pt)(2);

    for (unsigned int mnn = 0; mnn < master_node_neighbors.size(); ++mnn)
    {
      const Elem * master_neigh = master_node_neighbors[mnn];
      Point opposite = (master_neigh->node_ptr(0) == master_node) ? master_neigh->point(1)
                                                                  : master_neigh->point(0);
      Point cp = nodal_normal.cross(opposite - new_pt);
      master_node_cps[mnn] = cp(2);
    }

    // We will verify that only 1 orientation is actually valid.
    bool orientation1_valid = false, orientation2_valid = false;

    if (master_node_neighbors.size() == 2)
    {
      // 2 master neighbor case
      orientation1_valid = (slave_node_cps[0] * master_node_cps[0] > 0.) &&
                           (slave_node_cps[1] * master_node_cps[1] > 0.);

      orientation2_valid = (slave_node_cps[0] * master_node_cps[1] > 0.) &&
                           (slave_node_cps[1] * master_node_cps[0] > 0.);
    }
    else
    {
      // 1 master neighbor case
      orientation1_valid = (slave_node_cps[0] * master_node_cps[0] > 0.);
      orientation2_valid = (slave_node_cps[1] * master_node_cps[0] > 0.);
    }

    // Verify that both orientations are not simultaneously valid/invalid.
    if (orientation1_valid && orientation2_valid)
      libmesh_error_msg("Both orientations cannot simultaneously be valid.");
    if (!orientation1_valid && !orientation2_valid)
      libmesh_error_msg("Both orientations cannot simultaneously be invalid.");

    // If orientation 2 was valid, swap the left and right masters.
    if (orientation2_valid)
      std::swap(left_master_elem, right_master_elem);

    // Now that we know left_master_elem and right_master_elem, we can determine left_xi2 and
    // right_xi2.
    if (left_master_elem)
      left_xi2 = (master_node == left_master_elem->node_ptr(0)) ? -1 : +1;
    if (right_master_elem)
      right_xi2 = (master_node == right_master_elem->node_ptr(0)) ? -1 : +1;

    // Grab the MortarSegmentInfo object associated with this
    // segment. We can use "at()" here since we want this to fail if
    // current_mortar_segment is not found... Since we're going to
    // erase this entry from the map momentarily, we make an actual
    // copy rather than grabbing a reference.
    auto msm_it = msm_elem_to_info.find(current_mortar_segment);
    if (msm_it == msm_elem_to_info.end())
      libmesh_error_msg("MortarSegmentInfo not found for current_mortar_segment.");
    MortarSegmentInfo current_msinfo = msm_it->second;

    // Create new MortarSegmentInfo objects for new_elem_left and new_elem_right.
    MortarSegmentInfo new_msinfo_left, new_msinfo_right;

    // The new MortarSegmentInfo info objects inherit their "outer"
    // information from current_msinfo and the rest is determined by
    // the Node being inserted.
    new_msinfo_left.xi1_a = current_msinfo.xi1_a;
    new_msinfo_left.xi2_a = current_msinfo.xi2_a;
    new_msinfo_left.slave_elem = slave_elem;
    new_msinfo_left.xi1_b = xi1;
    new_msinfo_left.xi2_b = left_xi2;
    new_msinfo_left.master_elem = left_master_elem;

    new_msinfo_right.xi1_b = current_msinfo.xi1_b;
    new_msinfo_right.xi2_b = current_msinfo.xi2_b;
    new_msinfo_right.slave_elem = slave_elem;
    new_msinfo_right.xi1_a = xi1;
    new_msinfo_right.xi2_a = right_xi2;
    new_msinfo_right.master_elem = right_master_elem;

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
    // the mortar_segment_set for this slave_elem.
    mortar_segment_set.insert(new_elem_left);
    mortar_segment_set.insert(new_elem_right);
  }

  // Debugging: check the contents of slave_elems_to_mortar_segments.
  // libMesh::out << "\nAfter building the mortar segments." << std::endl;
  // for (const auto & pr : slave_elems_to_mortar_segments)
  //   {
  //     libMesh::out << "Mortar segments contained within slave elem " << pr.first->id() << ": ";
  //     for (const auto & elem : pr.second)
  //       libMesh::out << elem->id() << " ";
  //     libMesh::out << std::endl;
  //   }

  // Set up the the mortar segment neighbor information.
  mortar_segment_mesh.allow_renumbering(false);
  mortar_segment_mesh.prepare_for_use();

  // {
  //   // Temporary container to sort MortarSegmentInfo objects by
  //   // slave_elem to make them easier to interpret while printing.
  //   std::multimap<const Elem *, MortarSegmentInfo> sorted_msinfos;
  //   for (const auto & pr : msm_elem_to_info)
  //     sorted_msinfos.insert(std::make_pair(pr.second.slave_elem, pr.second));
  //
  //   libMesh::out << "\nPrinting info for *new* MortarSegmentMesh:" << std::endl;
  //   for (const auto & pr : sorted_msinfos)
  //     {
  //       pr.second.print();
  //
  //       // Check for validity of mortar segment.
  //       if (!pr.second.is_valid())
  //         libmesh_error_msg("Error! Mortar segment is not valid!");
  //     }
  // }

  // (Optionally) Write the mortar segment mesh to file for inspection
  {
    ExodusII_IO mortar_segment_mesh_writer(mortar_segment_mesh);
    mortar_segment_mesh_writer.write("mortar_segment_mesh.e");
  }

  // Loop over the msm_elem_to_info object and build a bi-directional
  // multimap from slave elements to the master Elems which they are
  // coupled to and vice-versa. This is used in the
  // AugmentSparsityOnInterface functor to determine whether a given
  // slave Elem is coupled across the mortar interface to a master
  // element.
  for (const auto & pr : msm_elem_to_info)
  {
    const Elem * slave_elem = pr.second.slave_elem;
    const Elem * master_elem = pr.second.master_elem;

    // The interior_parent() element should also be coupled to the
    // slave_elem. The coupling in the other direction (slave_elem
    // -> slave_elem->interior_parent()) happens automatically.
    mortar_interface_coupling.insert(std::make_pair(slave_elem->interior_parent(), slave_elem));

    // Insert both Elems as key and value.
    if (master_elem)
    {
      mortar_interface_coupling.insert(std::make_pair(slave_elem, master_elem));
      mortar_interface_coupling.insert(std::make_pair(master_elem, slave_elem));

      // Also insert both interior parents as key and value.
      mortar_interface_coupling.insert(
          std::make_pair(slave_elem->interior_parent(), master_elem->interior_parent()));
      mortar_interface_coupling.insert(
          std::make_pair(master_elem->interior_parent(), slave_elem->interior_parent()));
    }
  }
}

void
AutomaticMortarGeneration::compute_nodal_normals()
{
  // The dimension according to Mesh::mesh_dimension().
  const unsigned int dim = mesh.mesh_dimension();

  // Build FEType objects for the different variables. This order and
  // family isn't that important because we are only using it for
  // geometric information...
  FEType nnx_fe_type(FIRST, LAGRANGE);

  // Build FE objects from those types.
  UniquePtr<FEBase> nnx_fe_face(FEBase::build(dim, nnx_fe_type));

  // A lower-dimensional trapezoidal quadrature rule to be used on
  // faces.  We currently use a 1-point quadrature rule and compute a
  // single outward normal per (flat) face.
  QGauss qface(dim - 1, CONSTANT);

  // Tell the FE objects about the quadrature rule.
  nnx_fe_face->attach_quadrature_rule(&qface);

  // Get a reference to the normals from the face FE.
  const std::vector<Point> & face_normals = nnx_fe_face->get_normals();

  // Container to store the outward normal and length of each lower-dimensional slave element.
  std::unordered_map<const Elem *, std::pair<Point, Real>> slave_elem_to_normal_and_length;

  // First loop over lower-dimensional slave side elements and compute/save the outward normal for
  // each one. We loop over all active elements currently, but this procedure could be parallelized
  // as well.
  for (MeshBase::const_element_iterator el = mesh.active_elements_begin(),
                                        end_el = mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * slave_elem = *el;

    // If this is not one of the lower-dimensional slave side elements, go on to the next one.
    if (!this->slave_boundary_subdomain_ids.count(slave_elem->subdomain_id()))
      continue;

    // Which side of the parent are we? We need to know this to know
    // which side to reinit.
    const Elem * interior_parent = slave_elem->interior_parent();

    // Look up which side of the interior parent slave_elem is.
    unsigned int s = this->lower_elem_to_side_id.at(slave_elem);

    // libMesh::out << "In compute_nodal_normal(), found lower dimensional element "
    //              << slave_elem->id()
    //              << " with subdomain_id "
    //              << slave_elem->subdomain_id()
    //              << " which is side "
    //              << s
    //              << " of the interior parent."
    //              << std::endl;

    // Reinit the face FE object on side s.
    nnx_fe_face->reinit(interior_parent, s);

    // Store the outward normal at qp 0.
    slave_elem_to_normal_and_length[slave_elem] =
        std::make_pair(face_normals[0], slave_elem->volume());
  } // end 1st loop over lower-dimensional elements

  // Debugging: Print the geometric normals we have computed and stored thus far.
  // for (const auto & pr : slave_elem_to_normal_and_length)
  //   {
  //     const auto & value = pr.second;
  //     libMesh::out << "Slave elem " << pr.first->id() << " has geometric normal " << value.first
  //     << " and length " << value.second << std::endl;
  //   }

  // Now loop over the nodes_to_slave_elem_map and build the map from slave nodes to nodal normals.
  for (const auto & pr : nodes_to_slave_elem_map)
  {
    const auto & node_id = pr.first;
    const auto & nodal_neighbors = pr.second;
    // libMesh::out << "Computing nodal normal for node " << node_id << " with neighbor elements ";
    // for (const auto & elem : nodal_neighbors)
    //   libMesh::out << elem->id() << " ";
    // libMesh::out << std::endl;

    Real length_product = 1.;
    Point nodal_normal;

    // We will have either 2 or 1 nodal neighbors for each node,
    // this logic should handle either case.
    for (const auto & elem : nodal_neighbors)
    {
      // Find this elem in the slave_elem_to_normal_and_length, throwing an error if not found.
      const std::pair<Point, Real> & data = slave_elem_to_normal_and_length.at(elem);

      nodal_normal += data.second * data.first;
      length_product *= data.second;
    }

    // Divide the result by product of all the lengths.
    nodal_normal /= length_product;

    // Store the unit nodal normal vector.
    slave_node_to_nodal_normal[mesh.node_ptr(node_id)] = nodal_normal.unit();
  }

  // Debugging: print the results
  // for (const auto & pr : slave_node_to_nodal_normal)
  //   libMesh::out << "Slave node " << pr.first->id() << " has nodal normal " << pr.second <<
  //   std::endl;
}

// Project slave nodes onto their corresponding master elements for each master/slave pair.
void
AutomaticMortarGeneration::project_slave_nodes()
{
  // For each master/slave boundary id pair, call the
  // project_slave_nodes_single_pair() helper function.
  for (const auto & pr : master_slave_boundary_id_pairs)
    project_slave_nodes_single_pair(pr.first + boundary_subdomain_id_offset,
                                    pr.second + boundary_subdomain_id_offset);
}

void
AutomaticMortarGeneration::project_slave_nodes_single_pair(
    subdomain_id_type lower_dimensional_master_subdomain_id,
    subdomain_id_type lower_dimensional_slave_subdomain_id)
{
  // Build the "subdomain" adaptor based KD Tree.
  NanoflannMeshSubdomainAdaptor<3> mesh_adaptor(mesh, lower_dimensional_master_subdomain_id);
  subdomain_kd_tree_t kd_tree(
      3, mesh_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(/*max leaf=*/10));

  // Construct the KD tree.
  kd_tree.buildIndex();

  for (MeshBase::const_element_iterator el = mesh.active_elements_begin(),
                                        end_el = mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * slave_side_elem = *el;

    // If this Elem is not in the current slave subodmain, go on to the next one.
    if (slave_side_elem->subdomain_id() != lower_dimensional_slave_subdomain_id)
      continue;

    // For each node on the lower-dimensional element, find the nearest
    // node on the master side using the KDTree, then
    // search in nearby elements for where it projects
    // along the nodal normal direction.
    for (unsigned int n = 0; n < slave_side_elem->n_nodes(); ++n)
    {
      const Node * slave_node = slave_side_elem->node_ptr(n);

      // Get the nodal neighbors for slave_node, so we can check whether we've
      // already successfully projected it.
      const std::vector<const Elem *> & slave_node_neighbors =
          this->nodes_to_slave_elem_map.at(slave_node->id());

      // Check whether we've already mapped this slave node
      // successfully for all of its nodal neighbors.
      bool is_mapped = true;
      for (unsigned int snn = 0; snn < slave_node_neighbors.size(); ++snn)
      {
        auto slave_key = std::make_pair(slave_node, slave_node_neighbors[snn]);
        if (!slave_node_and_elem_to_xi2_master_elem.count(slave_key))
        {
          is_mapped = false;
          break;
        }
      }

      // Go to the next node if this one has already been mapped.
      if (is_mapped)
        continue;

      // Look up the new nodal normal value in the local storage, error if not found.
      Point nodal_normal = this->slave_node_to_nodal_normal.at(slave_node);

      // Data structure for performing Nanoflann searches.
      Real query_pt[3] = {(*slave_node)(0), (*slave_node)(1), (*slave_node)(2)};

      // The number of results we want to get.  We'll look for a
      // "few" nearest nodes, hopefully that is enough to let us
      // figure out which lower-dimensional Elem on the master
      // side we are across from.
      const size_t num_results = 3;

      // Initialize result_set and do the search.
      std::vector<size_t> ret_index(num_results);
      std::vector<Real> out_dist_sqr(num_results);
      nanoflann::KNNResultSet<Real> result_set(num_results);
      result_set.init(&ret_index[0], &out_dist_sqr[0]);
      kd_tree.findNeighbors(result_set, &query_pt[0], nanoflann::SearchParams(10));

      // If this flag gets set in the loop below, we can break out of the outer r-loop as well.
      bool projection_succeeded = false;

      // Once we've rejected a candidate for a given slave_node,
      // there's no reason to check it again.
      std::set<const Elem *> rejected_master_elem_candidates;

      // Loop over the closest nodes, check whether
      // the slave node successfully projects into
      // either of the closest neighbors, stop when
      // the projection succeeds.
      for (unsigned r = 0; r < result_set.size(); ++r)
      {
        // Verify that the squared distance we compute is the same as nanoflann's
        if (std::abs((mesh.point(ret_index[r]) - *slave_node).norm_sq() - out_dist_sqr[r]) >
            TOLERANCE)
          libmesh_error_msg("Lower-dimensional element squared distance verification failed.");

        // Get a reference to the vector of lower dimensional elements from the
        // nodes_to_master_elem_map.
        std::vector<const Elem *> & master_elem_candidates =
            this->nodes_to_master_elem_map.at(static_cast<dof_id_type>(ret_index[r]));

        // Search the Elems connected to this node on the master mesh side.
        for (unsigned int e = 0; e < master_elem_candidates.size(); ++e)
        {
          const Elem * master_elem_candidate = master_elem_candidates[e];

          // If we've already rejected this candidate, we don't need to check it again.
          if (rejected_master_elem_candidates.count(master_elem_candidate))
            continue;

          // Node positions of the potential master Elem.
          Point master1 = master_elem_candidate->point(0),
                master2 = master_elem_candidate->point(1);

          // Use variable names that more closely match the variable names in Bin Yang's
          // dissertation.
          Real master1_x = master1(0), master1_y = master1(1), master2_x = master2(0),
               master2_y = master2(1), slave_x = (*slave_node)(0), slave_y = (*slave_node)(1);

          // Use equation 2.4.5 from Bin Yang's dissertation to try and solve for the position on
          // the master element that the slave node projects to along the nodal normal. Note: we
          // have made the following simplifications to that equation in order to solve for xi: 1.)
          // Only the z (out-of-plane) component is solved for. 2.) We assume linear elements.
          Real numerator = -(master1_x + master2_x - 2. * slave_x) * nodal_normal(1) +
                           (master1_y + master2_y - 2. * slave_y) * nodal_normal(0);
          Real denominator = (-master1_x + master2_x) * nodal_normal(1) -
                             (-master1_y + master2_y) * nodal_normal(0);

          Real xi2 = numerator / denominator;
          // libMesh::out << "xi2=" << std::scientific << std::setprecision(16) << xi2 << std::endl;

          // Check whether the projection worked.
          if (std::abs(xi2) <= 1. + TOLERANCE)
          {
            // If xi2 == +1 or -1 then this slave node mapped directly to a node on the master
            // surface. This isn't as unlikely as you might think, it will happen if the meshes
            // on the interface start off being perfectly aligned. In this situation, we need to
            // associate the slave node with two different elements (and two corresponding xi^(2)
            // values.
            if (std::abs(std::abs(xi2) - 1.) < TOLERANCE)
            {
              const Node * master_node = (xi2 < 0) ? master_elem_candidate->node_ptr(0)
                                                   : master_elem_candidate->node_ptr(1);

              const std::vector<const Elem *> & master_node_neighbors =
                  this->nodes_to_master_elem_map.at(master_node->id());

              // Add entries to slave_node_and_elem_to_xi2_master_elem container.
              //
              // First, determine "on left" vs. "on right" orientation of the nodal neighbors. There
              // can be a max of 2 nodal neighbors, and we want to make sure that the slave nodal
              // neighbor on the "left" is associated with the master nodal neighbor on the "left"
              // and similarly for the "right".
              std::vector<Real> slave_node_neighbor_cps(2), master_node_neighbor_cps(2);

              // Figure out which slave side neighbor is on the "left" and which is on the "right".
              for (unsigned int nn = 0; nn < slave_node_neighbors.size(); ++nn)
              {
                const Elem * slave_neigh = slave_node_neighbors[nn];
                Point opposite = (slave_neigh->node_ptr(0) == slave_node) ? slave_neigh->point(1)
                                                                          : slave_neigh->point(0);
                Point cp = nodal_normal.cross(opposite - *slave_node);
                slave_node_neighbor_cps[nn] = cp(2);
              }

              // Figure out which master side neighbor is on the "left" and which is on the "right".
              for (unsigned int nn = 0; nn < master_node_neighbors.size(); ++nn)
              {
                const Elem * master_neigh = master_node_neighbors[nn];
                Point opposite = (master_neigh->node_ptr(0) == master_node)
                                     ? master_neigh->point(1)
                                     : master_neigh->point(0);
                Point cp = nodal_normal.cross(opposite - *slave_node);
                master_node_neighbor_cps[nn] = cp(2);
              }

              // Associate slave/master elems on matching sides.
              bool found_match = false;
              for (unsigned int snn = 0; snn < slave_node_neighbors.size(); ++snn)
                for (unsigned int mnn = 0; mnn < master_node_neighbors.size(); ++mnn)
                  if (slave_node_neighbor_cps[snn] * master_node_neighbor_cps[mnn] > 0)
                  {
                    found_match = true;

                    // Figure out xi^(2) value by looking at which node master_node is
                    // of the current master node neighbor.
                    Real xi2 = (master_node == master_node_neighbors[mnn]->node_ptr(0)) ? -1 : +1;
                    auto slave_key = std::make_pair(slave_node, slave_node_neighbors[snn]);
                    auto master_val = std::make_pair(xi2, master_node_neighbors[mnn]);
                    slave_node_and_elem_to_xi2_master_elem.insert(
                        std::make_pair(slave_key, master_val));

                    // Also map in the other direction.
                    Real xi1 = (slave_node == slave_node_neighbors[snn]->node_ptr(0)) ? -1 : +1;
                    auto master_key = std::make_pair(master_node, master_node_neighbors[mnn]);
                    auto slave_val = std::make_pair(xi1, slave_node_neighbors[snn]);
                    master_node_and_elem_to_xi1_slave_elem.insert(
                        std::make_pair(master_key, slave_val));

                    // Debugging
                    // libMesh::out << "Associating master node neighbor " << mnn
                    //              << " with slave node neighbor " << snn
                    //              << " xi^(2)= " << xi2
                    //              << ", and xi^(1)= " << xi1
                    //              << "." << std::endl;
                  }

              // Sanity check
              if (!found_match)
                libmesh_error_msg(
                    "Could not associate master/slave neighbors on either side of slave_node.");
            }
            else // Point falls somewhere in the middle of the Elem.
            {
              // Add two entries to slave_node_and_elem_to_xi2_master_elem.
              for (unsigned int nn = 0; nn < slave_node_neighbors.size(); ++nn)
              {
                const Elem * neigh = slave_node_neighbors[nn];
                for (unsigned int nid = 0; nid < neigh->n_nodes(); ++nid)
                {
                  const Node * neigh_node = neigh->node_ptr(nid);
                  if (slave_node == neigh_node)
                  {
                    auto key = std::make_pair(neigh_node, neigh);
                    auto val = std::make_pair(xi2, master_elem_candidate);
                    slave_node_and_elem_to_xi2_master_elem.insert(std::make_pair(key, val));
                  }
                }
              }
            }

            projection_succeeded = true;
            break; // out of e-loop
          }
          else
          {
            // The current slave_node is not in this Elem, so keep track of the rejects.
            rejected_master_elem_candidates.insert(master_elem_candidate);
          }
        }

        if (projection_succeeded)
          break; // out of r-loop
      }          // r-loop

      if (!projection_succeeded)
      {
        libMesh::out << "Failed to find master Elem into which slave node "
                     << static_cast<const Point &>(*slave_node) << " was projected." << std::endl;
      }
    } // loop over side nodes
  }   // end loop over lower-dimensional elements

  // Print contents of slave_node_and_elem_to_xi2_master_elem.
  // for (const auto & pr : slave_node_and_elem_to_xi2_master_elem)
  //   {
  //     auto key = pr.first;
  //     auto val = pr.second;
  //
  //     libMesh::out << "Key: Slave node id=" << key.first->id() << ", Slave elem id=" <<
  //     key.second->id() << std::endl; libMesh::out << "Val: xi^(2)=" << val.first << ", Master
  //     elem id=" << val.second->id() << std::endl;
  //   }
}

void
AutomaticMortarGeneration::write_nodal_normals_to_file()
{
  // Must call compute_nodal_normals() first!
  if (slave_node_to_nodal_normal.empty())
    libmesh_error_msg("No entries found in the slave node -> nodal normal map.");

  // Note: I seem to remember an issue with creating a second
  // EquationSystems with the same Mesh, but this code seems to work
  // OK currently.
  EquationSystems nodal_normals_es(this->mesh);
  ExplicitSystem & nodal_normals_system =
      nodal_normals_es.add_system<ExplicitSystem>("nodal_normals");
  unsigned int nnx_var_num =
                   nodal_normals_system.add_variable("nodal_normal_x", FEType(FIRST, LAGRANGE)),
               nny_var_num =
                   nodal_normals_system.add_variable("nodal_normal_y", FEType(FIRST, LAGRANGE));
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

    // For each node of the Elem, if it is in the slave_node_to_nodal_normal
    // container, set the corresponding nodal normal dof values.
    for (unsigned int n = 0; n < elem->n_nodes(); ++n)
    {
      auto it = this->slave_node_to_nodal_normal.find(elem->node_ptr(n));
      if (it != this->slave_node_to_nodal_normal.end())
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
