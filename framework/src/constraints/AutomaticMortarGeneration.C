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
#include "MortarSegmentHelper.h"
#include "FormattedTable.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "Output.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/explicit_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/dof_map.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"
#include "libmesh/face_tri7.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/face_quad9.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/quadrature_nodal.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/statistics.h"
#include "libmesh/equation_systems.h"

#include "metaphysicl/dualnumber.h"

#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

#include <array>
#include <algorithm>

using namespace libMesh;
using MetaPhysicL::DualNumber;

// Make newer nanoflann API spelling compatible with older nanoflann
// versions
#if NANOFLANN_VERSION < 0x150
namespace nanoflann
{
typedef SearchParams SearchParameters;
}
#endif

class MortarNodalGeometryOutput : public Output
{
public:
  static InputParameters validParams()
  {
    auto params = Output::validParams();
    params.addPrivateParam<AutomaticMortarGeneration *>("_amg", nullptr);
    params.addPrivateParam<MooseApp *>("_moose_app", nullptr);
    params.set<std::string>("_type") = "MortarNodalGeometryOutput";
    return params;
  };

  MortarNodalGeometryOutput(const InputParameters & params)
    : Output(params), _amg(*getCheckedPointerParam<AutomaticMortarGeneration *>("_amg"))
  {
  }

  void output() override
  {
    // Must call compute_nodal_geometry first!
    if (_amg._secondary_node_to_nodal_normal.empty() ||
        _amg._secondary_node_to_hh_nodal_tangents.empty())
      mooseError("No entries found in the secondary node -> nodal geometry map.");

    auto & problem = _app.feProblem();
    auto & subproblem = _amg._on_displaced
                            ? static_cast<SubProblem &>(*problem.getDisplacedProblem())
                            : static_cast<SubProblem &>(problem);
    auto & nodal_normals_es = subproblem.es();

    const std::string nodal_normals_sys_name = "nodal_normals";

    if (!_nodal_normals_system)
    {
      for (const auto s : make_range(nodal_normals_es.n_systems()))
        if (!nodal_normals_es.get_system(s).is_initialized())
          // This is really early on in the simulation and the systems have not been initialized. We
          // thus need to avoid calling reinit on systems that haven't even had their first init yet
          return;

      _nodal_normals_system =
          &nodal_normals_es.template add_system<ExplicitSystem>(nodal_normals_sys_name);
      _nnx_var_num = _nodal_normals_system->add_variable("nodal_normal_x", FEType(FIRST, LAGRANGE)),
      _nny_var_num = _nodal_normals_system->add_variable("nodal_normal_y", FEType(FIRST, LAGRANGE));
      _nnz_var_num = _nodal_normals_system->add_variable("nodal_normal_z", FEType(FIRST, LAGRANGE));

      _t1x_var_num =
          _nodal_normals_system->add_variable("nodal_tangent_1_x", FEType(FIRST, LAGRANGE)),
      _t1y_var_num =
          _nodal_normals_system->add_variable("nodal_tangent_1_y", FEType(FIRST, LAGRANGE));
      _t1z_var_num =
          _nodal_normals_system->add_variable("nodal_tangent_1_z", FEType(FIRST, LAGRANGE));

      _t2x_var_num =
          _nodal_normals_system->add_variable("nodal_tangent_2_x", FEType(FIRST, LAGRANGE)),
      _t2y_var_num =
          _nodal_normals_system->add_variable("nodal_tangent_2_y", FEType(FIRST, LAGRANGE));
      _t2z_var_num =
          _nodal_normals_system->add_variable("nodal_tangent_2_z", FEType(FIRST, LAGRANGE));
      nodal_normals_es.reinit();
    }

    const DofMap & dof_map = _nodal_normals_system->get_dof_map();
    std::vector<dof_id_type> dof_indices_nnx, dof_indices_nny, dof_indices_nnz;
    std::vector<dof_id_type> dof_indices_t1x, dof_indices_t1y, dof_indices_t1z;
    std::vector<dof_id_type> dof_indices_t2x, dof_indices_t2y, dof_indices_t2z;

    for (MeshBase::const_element_iterator el = _amg._mesh.elements_begin(),
                                          end_el = _amg._mesh.elements_end();
         el != end_el;
         ++el)
    {
      const Elem * elem = *el;

      // Get the nodal dofs for this Elem.
      dof_map.dof_indices(elem, dof_indices_nnx, _nnx_var_num);
      dof_map.dof_indices(elem, dof_indices_nny, _nny_var_num);
      dof_map.dof_indices(elem, dof_indices_nnz, _nnz_var_num);

      dof_map.dof_indices(elem, dof_indices_t1x, _t1x_var_num);
      dof_map.dof_indices(elem, dof_indices_t1y, _t1y_var_num);
      dof_map.dof_indices(elem, dof_indices_t1z, _t1z_var_num);

      dof_map.dof_indices(elem, dof_indices_t2x, _t2x_var_num);
      dof_map.dof_indices(elem, dof_indices_t2y, _t2y_var_num);
      dof_map.dof_indices(elem, dof_indices_t2z, _t2z_var_num);

      //

      // For each node of the Elem, if it is in the secondary_node_to_nodal_normal
      // container, set the corresponding nodal normal dof values.
      for (MooseIndex(elem->n_vertices()) n = 0; n < elem->n_vertices(); ++n)
      {
        auto it = _amg._secondary_node_to_nodal_normal.find(elem->node_ptr(n));
        if (it != _amg._secondary_node_to_nodal_normal.end())
        {
          _nodal_normals_system->solution->set(dof_indices_nnx[n], it->second(0));
          _nodal_normals_system->solution->set(dof_indices_nny[n], it->second(1));
          _nodal_normals_system->solution->set(dof_indices_nnz[n], it->second(2));
        }

        auto it_tangent = _amg._secondary_node_to_hh_nodal_tangents.find(elem->node_ptr(n));
        if (it_tangent != _amg._secondary_node_to_hh_nodal_tangents.end())
        {
          _nodal_normals_system->solution->set(dof_indices_t1x[n], it_tangent->second[0](0));
          _nodal_normals_system->solution->set(dof_indices_t1y[n], it_tangent->second[0](1));
          _nodal_normals_system->solution->set(dof_indices_t1z[n], it_tangent->second[0](2));

          _nodal_normals_system->solution->set(dof_indices_t2x[n], it_tangent->second[1](0));
          _nodal_normals_system->solution->set(dof_indices_t2y[n], it_tangent->second[1](1));
          _nodal_normals_system->solution->set(dof_indices_t2z[n], it_tangent->second[1](2));
        }

      } // end loop over nodes
    }   // end loop over elems

    // Finish assembly.
    _nodal_normals_system->solution->close();

    std::set<std::string> sys_names = {nodal_normals_sys_name};

    // Write the nodal normals to file
    ExodusII_IO nodal_normals_writer(_amg._mesh);

    // Default to non-HDF5 output for wider compatibility
    nodal_normals_writer.set_hdf5_writing(false);

    nodal_normals_writer.write_equation_systems(
        "nodal_geometry_only.e", nodal_normals_es, &sys_names);
  }

private:
  /// The mortar generation object that we will query for nodal normal and tangent information
  AutomaticMortarGeneration & _amg;

  ///@{
  /** Member variables for geometry debug output */
  libMesh::System * _nodal_normals_system = nullptr;
  unsigned int _nnx_var_num;
  unsigned int _nny_var_num;
  unsigned int _nnz_var_num;

  unsigned int _t1x_var_num;
  unsigned int _t1y_var_num;
  unsigned int _t1z_var_num;

  unsigned int _t2x_var_num;
  unsigned int _t2y_var_num;
  unsigned int _t2z_var_num;
  ///@}
};

AutomaticMortarGeneration::AutomaticMortarGeneration(
    MooseApp & app,
    MeshBase & mesh_in,
    const std::pair<BoundaryID, BoundaryID> & boundary_key,
    const std::pair<SubdomainID, SubdomainID> & subdomain_key,
    bool on_displaced,
    bool periodic,
    const bool debug,
    const bool correct_edge_dropping,
    const Real minimum_projection_angle)
  : ConsoleStreamInterface(app),
    _app(app),
    _mesh(mesh_in),
    _debug(debug),
    _on_displaced(on_displaced),
    _periodic(periodic),
    _distributed(!_mesh.is_replicated()),
    _correct_edge_dropping(correct_edge_dropping),
    _minimum_projection_angle(minimum_projection_angle)
{
  _primary_secondary_boundary_id_pairs.push_back(boundary_key);
  _primary_requested_boundary_ids.insert(boundary_key.first);
  _secondary_requested_boundary_ids.insert(boundary_key.second);
  _primary_secondary_subdomain_id_pairs.push_back(subdomain_key);
  _primary_boundary_subdomain_ids.insert(subdomain_key.first);
  _secondary_boundary_subdomain_ids.insert(subdomain_key.second);

  if (_distributed)
    _mortar_segment_mesh = std::make_unique<DistributedMesh>(_mesh.comm());
  else
    _mortar_segment_mesh = std::make_unique<ReplicatedMesh>(_mesh.comm());
}

void
AutomaticMortarGeneration::initOutput()
{
  if (!_debug)
    return;

  _output_params = std::make_unique<InputParameters>(MortarNodalGeometryOutput::validParams());
  _output_params->set<AutomaticMortarGeneration *>("_amg") = this;
  _output_params->set<FEProblemBase *>("_fe_problem_base") = &_app.feProblem();
  _output_params->set<MooseApp *>("_moose_app") = &_app;
  _output_params->set<std::string>("_object_name") =
      "mortar_nodal_geometry_" +
      std::to_string(_primary_secondary_boundary_id_pairs.front().first) +
      std::to_string(_primary_secondary_boundary_id_pairs.front().second) + "_" +
      (_on_displaced ? "displaced" : "undisplaced");
  _output_params->finalize("MortarNodalGeometryOutput");
  _app.getOutputWarehouse().addOutput(std::make_shared<MortarNodalGeometryOutput>(*_output_params));
}

void
AutomaticMortarGeneration::clear()
{
  _mortar_segment_mesh->clear();
  _nodes_to_secondary_elem_map.clear();
  _nodes_to_primary_elem_map.clear();
  _secondary_node_and_elem_to_xi2_primary_elem.clear();
  _primary_node_and_elem_to_xi1_secondary_elem.clear();
  _msm_elem_to_info.clear();
  _lower_elem_to_side_id.clear();
  _mortar_interface_coupling.clear();
  _secondary_node_to_nodal_normal.clear();
  _secondary_node_to_hh_nodal_tangents.clear();
  _secondary_element_to_secondary_lowerd_element.clear();
  _secondary_elems_to_mortar_segments.clear();
  _secondary_ip_sub_ids.clear();
  _primary_ip_sub_ids.clear();
}

void
AutomaticMortarGeneration::buildNodeToElemMaps()
{
  if (_secondary_requested_boundary_ids.empty() || _primary_requested_boundary_ids.empty())
    mooseError(
        "Must specify secondary and primary boundary ids before building node-to-elem maps.");

  // Construct nodes_to_secondary_elem_map
  for (const auto & secondary_elem :
       as_range(_mesh.active_elements_begin(), _mesh.active_elements_end()))
  {
    // If this is not one of the lower-dimensional secondary side elements, go on to the next one.
    if (!this->_secondary_boundary_subdomain_ids.count(secondary_elem->subdomain_id()))
      continue;

    for (const auto & nd : secondary_elem->node_ref_range())
    {
      std::vector<const Elem *> & vec = _nodes_to_secondary_elem_map[nd.id()];
      vec.push_back(secondary_elem);
    }
  }

  // Construct nodes_to_primary_elem_map
  for (const auto & primary_elem :
       as_range(_mesh.active_elements_begin(), _mesh.active_elements_end()))
  {
    // If this is not one of the lower-dimensional primary side elements, go on to the next one.
    if (!this->_primary_boundary_subdomain_ids.count(primary_elem->subdomain_id()))
      continue;

    for (const auto & nd : primary_elem->node_ref_range())
    {
      std::vector<const Elem *> & vec = _nodes_to_primary_elem_map[nd.id()];
      vec.push_back(primary_elem);
    }
  }
}

std::vector<Point>
AutomaticMortarGeneration::getNodalNormals(const Elem & secondary_elem) const
{
  std::vector<Point> nodal_normals(secondary_elem.n_nodes());
  for (const auto n : make_range(secondary_elem.n_nodes()))
    nodal_normals[n] = _secondary_node_to_nodal_normal.at(secondary_elem.node_ptr(n));

  return nodal_normals;
}

const Elem *
AutomaticMortarGeneration::getSecondaryLowerdElemFromSecondaryElem(
    dof_id_type secondary_elem_id) const
{
  mooseAssert(_secondary_element_to_secondary_lowerd_element.count(secondary_elem_id),
              "Map should locate secondary element");

  return _secondary_element_to_secondary_lowerd_element.at(secondary_elem_id);
}

std::map<unsigned int, unsigned int>
AutomaticMortarGeneration::getSecondaryIpToLowerElementMap(const Elem & lower_secondary_elem) const
{
  std::map<unsigned int, unsigned int> secondary_ip_i_to_lower_secondary_i;
  const Elem * const secondary_ip = lower_secondary_elem.interior_parent();
  mooseAssert(secondary_ip, "This should be non-null");

  for (const auto i : make_range(lower_secondary_elem.n_nodes()))
  {
    const auto & nd = lower_secondary_elem.node_ref(i);
    secondary_ip_i_to_lower_secondary_i[secondary_ip->get_node_index(&nd)] = i;
  }

  return secondary_ip_i_to_lower_secondary_i;
}

std::map<unsigned int, unsigned int>
AutomaticMortarGeneration::getPrimaryIpToLowerElementMap(
    const Elem & lower_primary_elem,
    const Elem & primary_elem,
    const Elem & /*lower_secondary_elem*/) const
{
  std::map<unsigned int, unsigned int> primary_ip_i_to_lower_primary_i;

  for (const auto i : make_range(lower_primary_elem.n_nodes()))
  {
    const auto & nd = lower_primary_elem.node_ref(i);
    primary_ip_i_to_lower_primary_i[primary_elem.get_node_index(&nd)] = i;
  }

  return primary_ip_i_to_lower_primary_i;
}

std::array<MooseUtils::SemidynamicVector<Point, 9>, 2>
AutomaticMortarGeneration::getNodalTangents(const Elem & secondary_elem) const
{
  // MetaPhysicL will check if we ran out of allocated space.
  MooseUtils::SemidynamicVector<Point, 9> nodal_tangents_one(0);
  MooseUtils::SemidynamicVector<Point, 9> nodal_tangents_two(0);

  for (const auto n : make_range(secondary_elem.n_nodes()))
  {
    const auto & tangent_vectors =
        libmesh_map_find(_secondary_node_to_hh_nodal_tangents, secondary_elem.node_ptr(n));
    nodal_tangents_one.push_back(tangent_vectors[0]);
    nodal_tangents_two.push_back(tangent_vectors[1]);
  }

  return {{nodal_tangents_one, nodal_tangents_two}};
}

std::vector<Point>
AutomaticMortarGeneration::getNormals(const Elem & secondary_elem,
                                      const std::vector<Real> & oned_xi1_pts) const
{
  std::vector<Point> xi1_pts(oned_xi1_pts.size());
  for (const auto qp : index_range(oned_xi1_pts))
    xi1_pts[qp] = oned_xi1_pts[qp];

  return getNormals(secondary_elem, xi1_pts);
}

std::vector<Point>
AutomaticMortarGeneration::getNormals(const Elem & secondary_elem,
                                      const std::vector<Point> & xi1_pts) const
{
  const auto mortar_dim = _mesh.mesh_dimension() - 1;
  const auto num_qps = xi1_pts.size();
  const auto nodal_normals = getNodalNormals(secondary_elem);
  std::vector<Point> normals(num_qps);

  for (const auto n : make_range(secondary_elem.n_nodes()))
    for (const auto qp : make_range(num_qps))
    {
      const auto phi =
          (mortar_dim == 1)
              ? Moose::fe_lagrange_1D_shape(secondary_elem.default_order(), n, xi1_pts[qp](0))
              : Moose::fe_lagrange_2D_shape(secondary_elem.type(),
                                            secondary_elem.default_order(),
                                            n,
                                            static_cast<const TypeVector<Real> &>(xi1_pts[qp]));
      normals[qp] += phi * nodal_normals[n];
    }

  if (_periodic)
    for (auto & normal : normals)
      normal *= -1;

  return normals;
}

void
AutomaticMortarGeneration::buildMortarSegmentMesh()
{
  dof_id_type local_id_index = 0;
  std::size_t node_unique_id_offset = 0;

  // Create an offset by the maximum number of mortar segment elements that can be created *plus*
  // the number of lower-dimensional secondary subdomain elements. Recall that the number of mortar
  // segments created is a function of node projection, *and* that if we split elems we will delete
  // that elem which has already taken a unique id
  for (const auto & pr : _primary_secondary_boundary_id_pairs)
  {
    const auto primary_bnd_id = pr.first;
    const auto secondary_bnd_id = pr.second;
    const auto num_primary_nodes =
        std::distance(_mesh.bid_nodes_begin(primary_bnd_id), _mesh.bid_nodes_end(primary_bnd_id));
    const auto num_secondary_nodes = std::distance(_mesh.bid_nodes_begin(secondary_bnd_id),
                                                   _mesh.bid_nodes_end(secondary_bnd_id));
    mooseAssert(num_primary_nodes,
                "There are no primary nodes on boundary ID "
                    << primary_bnd_id << ". Does that bondary ID even exist on the mesh?");
    mooseAssert(num_secondary_nodes,
                "There are no secondary nodes on boundary ID "
                    << secondary_bnd_id << ". Does that bondary ID even exist on the mesh?");

    node_unique_id_offset += num_primary_nodes + 2 * num_secondary_nodes;
  }

  // 1.) Add all lower-dimensional secondary side elements as the "initial" mortar segments.
  for (MeshBase::const_element_iterator el = _mesh.active_elements_begin(),
                                        end_el = _mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * secondary_elem = *el;

    // If this is not one of the lower-dimensional secondary side elements, go on to the next one.
    if (!this->_secondary_boundary_subdomain_ids.count(secondary_elem->subdomain_id()))
      continue;

    std::vector<Node *> new_nodes;
    for (MooseIndex(secondary_elem->n_nodes()) n = 0; n < secondary_elem->n_nodes(); ++n)
    {
      new_nodes.push_back(_mortar_segment_mesh->add_point(
          secondary_elem->point(n), secondary_elem->node_id(n), secondary_elem->processor_id()));
      Node * const new_node = new_nodes.back();
      new_node->set_unique_id(new_node->id() + node_unique_id_offset);
    }

    std::unique_ptr<Elem> new_elem;
    if (secondary_elem->default_order() == SECOND)
      new_elem = std::make_unique<Edge3>();
    else
      new_elem = std::make_unique<Edge2>();

    new_elem->processor_id() = secondary_elem->processor_id();
    new_elem->subdomain_id() = secondary_elem->subdomain_id();
    new_elem->set_id(local_id_index++);
    new_elem->set_unique_id(new_elem->id());

    for (MooseIndex(new_elem->n_nodes()) n = 0; n < new_elem->n_nodes(); ++n)
      new_elem->set_node(n) = new_nodes[n];

    Elem * new_elem_ptr = _mortar_segment_mesh->add_elem(new_elem.release());

    // The xi^(1) values for this mortar segment are initially -1 and 1.
    MortarSegmentInfo msinfo;
    msinfo.xi1_a = -1;
    msinfo.xi1_b = +1;
    msinfo.secondary_elem = secondary_elem;

    auto new_container_it0 = _secondary_node_and_elem_to_xi2_primary_elem.find(
             std::make_pair(secondary_elem->node_ptr(0), secondary_elem)),
         new_container_it1 = _secondary_node_and_elem_to_xi2_primary_elem.find(
             std::make_pair(secondary_elem->node_ptr(1), secondary_elem));

    bool new_container_node0_found =
             (new_container_it0 != _secondary_node_and_elem_to_xi2_primary_elem.end()),
         new_container_node1_found =
             (new_container_it1 != _secondary_node_and_elem_to_xi2_primary_elem.end());

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
    _msm_elem_to_info.emplace(new_elem_ptr, msinfo);

    // Maintain the mapping between secondary elems and mortar segment elems contained within them.
    // Initially, only the original secondary_elem is present.
    _secondary_elems_to_mortar_segments[secondary_elem->id()].insert(new_elem_ptr);
  }

  // 2.) Insert new nodes from primary side and split mortar segments as necessary.
  for (const auto & pr : _primary_node_and_elem_to_xi1_secondary_elem)
  {
    auto key = pr.first;
    auto val = pr.second;

    const Node * primary_node = std::get<1>(key);
    Real xi1 = val.first;
    const Elem * secondary_elem = val.second;

    // If this is an aligned node, we don't need to do anything.
    if (std::abs(std::abs(xi1) - 1.) < _xi_tolerance)
      continue;

    auto && order = secondary_elem->default_order();

    // Determine physical location of new point to be inserted.
    Point new_pt(0);
    for (MooseIndex(secondary_elem->n_nodes()) n = 0; n < secondary_elem->n_nodes(); ++n)
      new_pt += Moose::fe_lagrange_1D_shape(order, n, xi1) * secondary_elem->point(n);

    // Find the current mortar segment that will have to be split.
    auto & mortar_segment_set = _secondary_elems_to_mortar_segments[secondary_elem->id()];
    Elem * current_mortar_segment = nullptr;
    MortarSegmentInfo * info = nullptr;

    for (const auto & mortar_segment_candidate : mortar_segment_set)
    {
      try
      {
        info = &_msm_elem_to_info.at(mortar_segment_candidate);
      }
      catch (std::out_of_range &)
      {
        mooseError("MortarSegmentInfo not found for the mortar segment candidate");
      }
      if (info->xi1_a <= xi1 && xi1 <= info->xi1_b)
      {
        current_mortar_segment = mortar_segment_candidate;
        break;
      }
    }

    // Make sure we found one.
    if (current_mortar_segment == nullptr)
      mooseError("Unable to find appropriate mortar segment during linear search!");

    // If node lands on endpoint of segment, don't split.
    // Jacob: This condition was getting missed by the < comparison a few lines above. To fix it I
    // just made it <= and put this condition in to handle equality different. It probably could be
    // done with a tolerance but the the toleranced equality is already handled later when we drop
    // segments with small volume.
    if (info->xi1_a == xi1 || xi1 == info->xi1_b)
      continue;

    const auto new_id = _mortar_segment_mesh->max_node_id() + 1;
    mooseAssert(_mortar_segment_mesh->comm().verify(new_id),
                "new_id must be the same on all processes");
    Node * const new_node =
        _mortar_segment_mesh->add_point(new_pt, new_id, secondary_elem->processor_id());
    new_node->set_unique_id(new_id + node_unique_id_offset);

    // Reconstruct the nodal normal at xi1. This will help us
    // determine the orientation of the primary elems relative to the
    // new mortar segments.
    const Point normal = getNormals(*secondary_elem, std::vector<Real>({xi1}))[0];

    // Get the set of primary_node neighbors.
    if (this->_nodes_to_primary_elem_map.find(primary_node->id()) ==
        this->_nodes_to_primary_elem_map.end())
      mooseError("We should already have built this primary node to elem pair!");
    const std::vector<const Elem *> & primary_node_neighbors =
        this->_nodes_to_primary_elem_map[primary_node->id()];

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
    std::array<Real, 2> secondary_node_cps;
    std::vector<Real> primary_node_cps(primary_node_neighbors.size());

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
    else if (primary_node_neighbors.size() == 1)
    {
      // 1 primary neighbor case
      orientation1_valid = (secondary_node_cps[0] * primary_node_cps[0] > 0.);
      orientation2_valid = (secondary_node_cps[1] * primary_node_cps[0] > 0.);
    }
    else
      mooseError("Invalid primary node neighbors size ", primary_node_neighbors.size());

    // Verify that both orientations are not simultaneously valid/invalid. If they are not, then we
    // are going to throw an exception instead of erroring out since we can easily reach this point
    // if we have one bad linear solve. It's better in general to catch the error and then try a
    // smaller time-step
    if (orientation1_valid && orientation2_valid)
      throw MooseException(
          "AutomaticMortarGeneration: Both orientations cannot simultaneously be valid.");

    // We are going to treat the case where both orientations are invalid as a case in which we
    // should not be splitting the mortar mesh to incorporate primary mesh elements.
    // In practice, this case has appeared for very oblique projections, so we assume these cases
    // will not be considered in mortar thermomechanical contact.
    if (!orientation1_valid && !orientation2_valid)
    {
      mooseDoOnce(mooseWarning(
          "AutomaticMortarGeneration: Unable to determine valid secondary-primary orientation. "
          "Consequently we will consider projection of the primary node invalid and not split the "
          "mortar segment. "
          "This situation can indicate there are very oblique projections between primary (mortar) "
          "and secondary (non-mortar) surfaces for a good problem set up. It can also mean your "
          "time step is too large. This message is only printed once."));
      continue;
    }

    // Make an Elem on the left
    std::unique_ptr<Elem> new_elem_left;
    if (order == SECOND)
      new_elem_left = std::make_unique<Edge3>();
    else
      new_elem_left = std::make_unique<Edge2>();

    new_elem_left->processor_id() = current_mortar_segment->processor_id();
    new_elem_left->subdomain_id() = current_mortar_segment->subdomain_id();
    new_elem_left->set_id(local_id_index++);
    new_elem_left->set_unique_id(new_elem_left->id());
    new_elem_left->set_node(0) = current_mortar_segment->node_ptr(0);
    new_elem_left->set_node(1) = new_node;

    // Make an Elem on the right
    std::unique_ptr<Elem> new_elem_right;
    if (order == SECOND)
      new_elem_right = std::make_unique<Edge3>();
    else
      new_elem_right = std::make_unique<Edge2>();

    new_elem_right->processor_id() = current_mortar_segment->processor_id();
    new_elem_right->subdomain_id() = current_mortar_segment->subdomain_id();
    new_elem_right->set_id(local_id_index++);
    new_elem_right->set_unique_id(new_elem_right->id());
    new_elem_right->set_node(0) = new_node;
    new_elem_right->set_node(1) = current_mortar_segment->node_ptr(1);

    if (order == SECOND)
    {
      // left
      Point left_interior_point(0);
      Real left_interior_xi = (xi1 + info->xi1_a) / 2;

      // This is eta for the current mortar segment that we're splitting
      Real current_left_interior_eta =
          (2. * left_interior_xi - info->xi1_a - info->xi1_b) / (info->xi1_b - info->xi1_a);

      for (MooseIndex(current_mortar_segment->n_nodes()) n = 0;
           n < current_mortar_segment->n_nodes();
           ++n)
        left_interior_point += Moose::fe_lagrange_1D_shape(order, n, current_left_interior_eta) *
                               current_mortar_segment->point(n);

      const auto new_interior_left_id = _mortar_segment_mesh->max_node_id() + 1;
      mooseAssert(_mortar_segment_mesh->comm().verify(new_interior_left_id),
                  "new_id must be the same on all processes");
      Node * const new_interior_node_left = _mortar_segment_mesh->add_point(
          left_interior_point, new_interior_left_id, new_elem_left->processor_id());
      new_elem_left->set_node(2) = new_interior_node_left;
      new_interior_node_left->set_unique_id(new_interior_left_id + node_unique_id_offset);

      // right
      Point right_interior_point(0);
      Real right_interior_xi = (xi1 + info->xi1_b) / 2;
      // This is eta for the current mortar segment that we're splitting
      Real current_right_interior_eta =
          (2. * right_interior_xi - info->xi1_a - info->xi1_b) / (info->xi1_b - info->xi1_a);

      for (MooseIndex(current_mortar_segment->n_nodes()) n = 0;
           n < current_mortar_segment->n_nodes();
           ++n)
        right_interior_point += Moose::fe_lagrange_1D_shape(order, n, current_right_interior_eta) *
                                current_mortar_segment->point(n);

      const auto new_interior_id_right = _mortar_segment_mesh->max_node_id() + 1;
      mooseAssert(_mortar_segment_mesh->comm().verify(new_interior_id_right),
                  "new_id must be the same on all processes");
      Node * const new_interior_node_right = _mortar_segment_mesh->add_point(
          right_interior_point, new_interior_id_right, new_elem_right->processor_id());
      new_elem_right->set_node(2) = new_interior_node_right;
      new_interior_node_right->set_unique_id(new_interior_id_right + node_unique_id_offset);
    }

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
    auto msm_it = _msm_elem_to_info.find(current_mortar_segment);
    if (msm_it == _msm_elem_to_info.end())
      mooseError("MortarSegmentInfo not found for current_mortar_segment.");
    MortarSegmentInfo current_msinfo = msm_it->second;

    // add_left
    {
      Elem * msm_new_elem = _mortar_segment_mesh->add_elem(new_elem_left.release());

      // Create new MortarSegmentInfo objects for new_elem_left
      MortarSegmentInfo new_msinfo_left;

      // The new MortarSegmentInfo info objects inherit their "outer"
      // information from current_msinfo and the rest is determined by
      // the Node being inserted.
      new_msinfo_left.xi1_a = current_msinfo.xi1_a;
      new_msinfo_left.xi2_a = current_msinfo.xi2_a;
      new_msinfo_left.secondary_elem = secondary_elem;
      new_msinfo_left.xi1_b = xi1;
      new_msinfo_left.xi2_b = left_xi2;
      new_msinfo_left.primary_elem = left_primary_elem;

      // Add new msinfo objects to the map.
      _msm_elem_to_info.emplace(msm_new_elem, new_msinfo_left);

      // We need to insert new_elem_left in
      // the mortar_segment_set for this secondary_elem.
      mortar_segment_set.insert(msm_new_elem);
    }

    // add_right
    {
      Elem * msm_new_elem = _mortar_segment_mesh->add_elem(new_elem_right.release());

      // Create new MortarSegmentInfo objects for new_elem_right
      MortarSegmentInfo new_msinfo_right;

      new_msinfo_right.xi1_b = current_msinfo.xi1_b;
      new_msinfo_right.xi2_b = current_msinfo.xi2_b;
      new_msinfo_right.secondary_elem = secondary_elem;
      new_msinfo_right.xi1_a = xi1;
      new_msinfo_right.xi2_a = right_xi2;
      new_msinfo_right.primary_elem = right_primary_elem;

      _msm_elem_to_info.emplace(msm_new_elem, new_msinfo_right);

      mortar_segment_set.insert(msm_new_elem);
    }

    // Erase the MortarSegmentInfo object for current_mortar_segment from the map.
    _msm_elem_to_info.erase(msm_it);

    // current_mortar_segment must be erased from the
    // mortar_segment_set since it has now been split.
    mortar_segment_set.erase(current_mortar_segment);

    // The original mortar segment has been split, so erase it from
    // the mortar segment mesh.
    _mortar_segment_mesh->delete_elem(current_mortar_segment);
  }

  // Remove all MSM elements without a primary contribution
  /**
   * This was a change to how inactive LM DoFs are handled. Now mortar segment elements
   * are not used in assembly if there is no corresponding primary element and inactive
   * LM DoFs (those with no contribution to an active primary element) are zeroed.
   */
  for (auto msm_elem : _mortar_segment_mesh->active_element_ptr_range())
  {
    MortarSegmentInfo & msinfo = libmesh_map_find(_msm_elem_to_info, msm_elem);
    Elem * primary_elem = const_cast<Elem *>(msinfo.primary_elem);
    if (primary_elem == nullptr || std::abs(msinfo.xi2_a) > 1.0 + TOLERANCE ||
        std::abs(msinfo.xi2_b) > 1.0 + TOLERANCE)
    {
      // Erase from secondary to msms map
      auto it = _secondary_elems_to_mortar_segments.find(msinfo.secondary_elem->id());
      mooseAssert(it != _secondary_elems_to_mortar_segments.end(),
                  "We should have found the element");
      auto & msm_set = it->second;
      msm_set.erase(msm_elem);
      // We may be creating nodes with only one element neighbor where before this removal there
      // were two. But the nodal normal used in computations will reflect the two-neighbor geometry.
      // For a lower-d secondary mesh corner, that will imply the corner node will have a tilted
      // normal vector (same for tangents) despite the mortar segment mesh not including its
      // vertical neighboring element. It is the secondary element neighbors (not mortar segment
      // mesh neighbors) that determine the nodal normal field.
      if (msm_set.empty())
        _secondary_elems_to_mortar_segments.erase(it);

      // Erase msinfo
      _msm_elem_to_info.erase(msm_elem);

      // Remove element from mortar segment mesh
      _mortar_segment_mesh->delete_elem(msm_elem);
    }
    else
    {
      _secondary_ip_sub_ids.insert(msinfo.secondary_elem->interior_parent()->subdomain_id());
      _primary_ip_sub_ids.insert(msinfo.primary_elem->interior_parent()->subdomain_id());
    }
  }

  std::unordered_set<Node *> msm_connected_nodes;

  // Deleting elements may produce isolated nodes.
  // Loops for identifying and removing such nodes from mortar segment mesh.
  for (const auto & element : _mortar_segment_mesh->element_ptr_range())
    for (auto & n : element->node_ref_range())
      msm_connected_nodes.insert(&n);

  for (const auto & node : _mortar_segment_mesh->node_ptr_range())
    if (!msm_connected_nodes.count(node))
      _mortar_segment_mesh->delete_node(node);

#ifdef DEBUG
  // Verify that all segments without primary contribution have been deleted
  for (auto msm_elem : _mortar_segment_mesh->active_element_ptr_range())
  {
    const MortarSegmentInfo & msinfo = libmesh_map_find(_msm_elem_to_info, msm_elem);
    mooseAssert(msinfo.primary_elem != nullptr,
                "All mortar segment elements should have valid "
                "primary element.");
  }
#endif

  _mortar_segment_mesh->cache_elem_data();

  // (Optionally) Write the mortar segment mesh to file for inspection
  if (_debug)
  {
    ExodusII_IO mortar_segment_mesh_writer(*_mortar_segment_mesh);

    // Default to non-HDF5 output for wider compatibility
    mortar_segment_mesh_writer.set_hdf5_writing(false);

    mortar_segment_mesh_writer.write("mortar_segment_mesh.e");
  }

  buildCouplingInformation();
}

void
AutomaticMortarGeneration::buildMortarSegmentMesh3d()
{
  // Add an integer flag to mortar segment mesh to keep track of which subelem
  // of second order primal elements mortar segments correspond to
  auto secondary_sub_elem = _mortar_segment_mesh->add_elem_integer("secondary_sub_elem");
  auto primary_sub_elem = _mortar_segment_mesh->add_elem_integer("primary_sub_elem");

  dof_id_type local_id_index = 0;

  // Loop through mortar secondary and primary pairs to create mortar segment mesh between each
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
  {
    const auto primary_subd_id = pr.first;
    const auto secondary_subd_id = pr.second;

    // Build k-d tree for use in Step 1.2 for primary interface coarse screening
    NanoflannMeshSubdomainAdaptor<3> mesh_adaptor(_mesh, primary_subd_id);
    subdomain_kd_tree_t kd_tree(
        3, mesh_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(/*max leaf=*/10));

    // Construct the KD tree.
    kd_tree.buildIndex();

    // Define expression for getting sub-elements nodes (for sub-dividing secondary elements)
    auto get_sub_elem_nodes = [](const ElemType type,
                                 const unsigned int sub_elem) -> std::vector<unsigned int>
    {
      switch (type)
      {
        case TRI3:
          return {{0, 1, 2}};
        case QUAD4:
          return {{0, 1, 2, 3}};
        case TRI6:
        case TRI7:
          switch (sub_elem)
          {
            case 0:
              return {{0, 3, 5}};
            case 1:
              return {{3, 4, 5}};
            case 2:
              return {{3, 1, 4}};
            case 3:
              return {{5, 4, 2}};
            default:
              mooseError("get_sub_elem_nodes: Invalid sub_elem: ", sub_elem);
          }
        case QUAD8:
          switch (sub_elem)
          {
            case 0:
              return {{0, 4, 7}};
            case 1:
              return {{4, 1, 5}};
            case 2:
              return {{5, 2, 6}};
            case 3:
              return {{7, 6, 3}};
            case 4:
              return {{4, 5, 6, 7}};
            default:
              mooseError("get_sub_elem_nodes: Invalid sub_elem: ", sub_elem);
          }
        case QUAD9:
          switch (sub_elem)
          {
            case 0:
              return {{0, 4, 8, 7}};
            case 1:
              return {{4, 1, 5, 8}};
            case 2:
              return {{8, 5, 2, 6}};
            case 3:
              return {{7, 8, 6, 3}};
            default:
              mooseError("get_sub_elem_nodes: Invalid sub_elem: ", sub_elem);
          }
        default:
          mooseError("get_sub_elem_inds: Face element type: ",
                     libMesh::Utility::enum_to_string<ElemType>(type),
                     " invalid for 3D mortar");
      }
    };

    /**
     *  Step 1: Build mortar segments for all secondary elements
     */
    for (MeshBase::const_element_iterator el = _mesh.active_local_elements_begin(),
                                          end_el = _mesh.active_local_elements_end();
         el != end_el;
         ++el)
    {
      const Elem * secondary_side_elem = *el;

      const Real secondary_volume = secondary_side_elem->volume();

      // If this Elem is not in the current secondary subdomain, go on to the next one.
      if (secondary_side_elem->subdomain_id() != secondary_subd_id)
        continue;

      auto [secondary_elem_to_msm_map_it, insertion_happened] =
          _secondary_elems_to_mortar_segments.emplace(secondary_side_elem->id(),
                                                      std::set<Elem *, CompareDofObjectsByID>{});
      libmesh_ignore(insertion_happened);
      auto & secondary_to_msm_element_set = secondary_elem_to_msm_map_it->second;

      std::vector<std::unique_ptr<MortarSegmentHelper>> mortar_segment_helper(
          secondary_side_elem->n_sub_elem());
      const auto nodal_normals = getNodalNormals(*secondary_side_elem);

      /**
       * Step 1.1: Linearize secondary face elements
       *
       * For first order face elements (Tri3 and Quad4) elements are simply linearized around center
       * For second order (Tri6 and Quad9) and third order (Tri7) face elements, elements are
       * sub-divided into four first order elements then each of the sub-elements is linearized
       * around their respective centers
       * For Quad8 elements, they are sub-divided into one quad and four triangle elements and each
       * sub-element is linearized around their respective centers
       */
      for (auto sel : make_range(secondary_side_elem->n_sub_elem()))
      {
        // Get indices of sub-element nodes in element
        auto sub_elem_nodes = get_sub_elem_nodes(secondary_side_elem->type(), sel);

        // Secondary sub-element center, normal, and nodes
        Point center;
        Point normal;
        std::vector<Point> nodes(sub_elem_nodes.size());

        // Loop through sub_element nodes, collect points and compute center and normal
        for (auto iv : make_range(sub_elem_nodes.size()))
        {
          const auto n = sub_elem_nodes[iv];
          nodes[iv] = secondary_side_elem->point(n);
          center += secondary_side_elem->point(n);
          normal += nodal_normals[n];
        }
        center /= sub_elem_nodes.size();
        normal = normal.unit();

        // Build and store linearized sub-elements for later use
        mortar_segment_helper[sel] = std::make_unique<MortarSegmentHelper>(nodes, center, normal);
      }

      /**
       * Step 1.2: Coarse screening using a k-d tree to find nodes on the primary interface that are
       *    'close to' a center point of the secondary element.
       */

      // Search point for performing Nanoflann (k-d tree) searches.
      // In each case we use the center point of the original element (not sub-elements for second
      // order elements). This is to do search for all sub-elements simultaneously
      std::array<Real, 3> query_pt;
      Point center_point;
      switch (secondary_side_elem->type())
      {
        case TRI3:
        case QUAD4:
          center_point = mortar_segment_helper[0]->center();
          query_pt = {{center_point(0), center_point(1), center_point(2)}};
          break;
        case TRI6:
        case TRI7:
          center_point = mortar_segment_helper[1]->center();
          query_pt = {{center_point(0), center_point(1), center_point(2)}};
          break;
        case QUAD8:
          center_point = mortar_segment_helper[4]->center();
          query_pt = {{center_point(0), center_point(1), center_point(2)}};
          break;
        case QUAD9:
          center_point = secondary_side_elem->point(8);
          query_pt = {{center_point(0), center_point(1), center_point(2)}};
          break;
        default:
          mooseError(
              "Face element type: ", secondary_side_elem->type(), "not supported for 3D mortar");
      }

      // The number of results we want to get. These results will only be used to find
      // a single element with non-trivial overlap, after an element is identified a breadth
      // first search is done on neighbors
      const std::size_t num_results = 3;

      // Initialize result_set and do the search.
      std::vector<size_t> ret_index(num_results);
      std::vector<Real> out_dist_sqr(num_results);
      nanoflann::KNNResultSet<Real> result_set(num_results);
      result_set.init(&ret_index[0], &out_dist_sqr[0]);
      kd_tree.findNeighbors(result_set, &query_pt[0], nanoflann::SearchParameters());

      // Initialize list of processed primary elements, we don't want to revisit processed elements
      std::set<const Elem *, CompareDofObjectsByID> processed_primary_elems;

      // Initialize candidate set and flag for switching between coarse screening and breadth-first
      // search
      bool primary_elem_found = false;
      std::set<const Elem *, CompareDofObjectsByID> primary_elem_candidates;

      // Loop candidate nodes (returned by Nanoflann) and add all adjoining elems to candidate set
      for (auto r : make_range(result_set.size()))
      {
        // Verify that the squared distance we compute is the same as nanoflann's
        mooseAssert(std::abs((_mesh.point(ret_index[r]) - center_point).norm_sq() -
                             out_dist_sqr[r]) <= TOLERANCE,
                    "Lower-dimensional element squared distance verification failed.");

        // Get list of elems connected to node
        std::vector<const Elem *> & node_elems =
            this->_nodes_to_primary_elem_map.at(static_cast<dof_id_type>(ret_index[r]));

        // Uniquely add elems to candidate set
        for (auto elem : node_elems)
          primary_elem_candidates.insert(elem);
      }

      /**
       * Step 1.3: Loop through primary candidate nodes, create mortar segments
       *
       * Once an element with non-trivial projection onto secondary element identified, switch
       * to breadth-first search (drop all current candidates and add only neighbors of elements
       * with non-trivial overlap)
       */
      while (!primary_elem_candidates.empty())
      {
        const Elem * primary_elem_candidate = *primary_elem_candidates.begin();

        // If we've already processed this candidate, we don't need to check it again.
        if (processed_primary_elems.count(primary_elem_candidate))
          continue;

        // Initialize set of nodes used to construct mortar segment elements
        std::vector<Point> nodal_points;

        // Initialize map from mortar segment elements to nodes
        std::vector<std::vector<unsigned int>> elem_to_node_map;

        // Initialize list of secondary and primary sub-elements that formed each mortar segment
        std::vector<std::pair<unsigned int, unsigned int>> sub_elem_map;

        /**
         * Step 1.3.2: Sub-divide primary element candidate, then project onto secondary
         * sub-elements, perform polygon clipping, and triangulate to form mortar segments
         */
        for (auto p_el : make_range(primary_elem_candidate->n_sub_elem()))
        {
          // Get nodes of primary sub-elements
          auto sub_elem_nodes = get_sub_elem_nodes(primary_elem_candidate->type(), p_el);

          // Get list of primary sub-element vertex nodes
          std::vector<Point> primary_sub_elem(sub_elem_nodes.size());
          for (auto iv : make_range(sub_elem_nodes.size()))
          {
            const auto n = sub_elem_nodes[iv];
            primary_sub_elem[iv] = primary_elem_candidate->point(n);
          }

          // Loop through secondary sub-elements
          for (auto s_el : make_range(secondary_side_elem->n_sub_elem()))
          {
            // Mortar segment helpers were defined for each secondary sub-element, they will:
            //  1. Project primary sub-element onto linearized secondary sub-element
            //  2. Clip projected primary sub-element against secondary sub-element
            //  3. Triangulate clipped polygon to form mortar segments
            //
            // Mortar segment helpers append a list of mortar segment nodes and connectivities that
            // can be directly used to build mortar segments
            mortar_segment_helper[s_el]->getMortarSegments(
                primary_sub_elem, nodal_points, elem_to_node_map);

            // Keep track of which secondary and primary sub-elements created segment
            for (auto i = sub_elem_map.size(); i < elem_to_node_map.size(); ++i)
              sub_elem_map.push_back(std::make_pair(s_el, p_el));
          }
        }

        // Mark primary element as processed and remove from candidate list
        processed_primary_elems.insert(primary_elem_candidate);
        primary_elem_candidates.erase(primary_elem_candidate);

        // If overlap of polygons was non-trivial (created mortar segment elements)
        if (!elem_to_node_map.empty())
        {
          // If this is the first element with non-trivial overlap, set flag
          // Candidates will now be neighbors of elements that had non-trivial overlap
          // (i.e. we'll do a breadth first search now)
          if (!primary_elem_found)
          {
            primary_elem_found = true;
            primary_elem_candidates.clear();
          }

          // Add neighbors to candidate list
          for (auto neighbor : primary_elem_candidate->neighbor_ptr_range())
          {
            // If not valid or not on lower dimensional secondary subdomain, skip
            if (neighbor == nullptr || neighbor->subdomain_id() != primary_subd_id)
              continue;
            // If already processed, skip
            if (processed_primary_elems.count(neighbor))
              continue;
            // Otherwise, add to candidates
            primary_elem_candidates.insert(neighbor);
          }

          /**
           * Step 1.3.3: Create mortar segments and add to mortar segment mesh
           */
          std::vector<Node *> new_nodes;
          for (auto pt : nodal_points)
            new_nodes.push_back(_mortar_segment_mesh->add_point(
                pt, _mortar_segment_mesh->max_node_id(), secondary_side_elem->processor_id()));

          // Loop through triangular elements in map
          for (auto el : index_range(elem_to_node_map))
          {
            // Create new triangular element
            std::unique_ptr<Elem> new_elem;
            if (elem_to_node_map[el].size() == 3)
              new_elem = std::make_unique<Tri3>();
            else
              mooseError("Active mortar segments only supports TRI elements, 3 nodes expected "
                         "but: ",
                         elem_to_node_map[el].size(),
                         " provided.");

            new_elem->processor_id() = secondary_side_elem->processor_id();
            new_elem->subdomain_id() = secondary_side_elem->subdomain_id();
            new_elem->set_id(local_id_index++);

            // Attach newly created nodes
            for (auto i : index_range(elem_to_node_map[el]))
              new_elem->set_node(i) = new_nodes[elem_to_node_map[el][i]];

            // If element is smaller than tolerance, don't add to msm
            if (new_elem->volume() / secondary_volume < TOLERANCE)
              continue;

            // Add elements to mortar segment mesh
            Elem * msm_new_elem = _mortar_segment_mesh->add_elem(new_elem.release());

            msm_new_elem->set_extra_integer(secondary_sub_elem, sub_elem_map[el].first);
            msm_new_elem->set_extra_integer(primary_sub_elem, sub_elem_map[el].second);

            // Fill out mortar segment info
            MortarSegmentInfo msinfo;
            msinfo.secondary_elem = secondary_side_elem;
            msinfo.primary_elem = primary_elem_candidate;

            // Associate this MSM elem with the MortarSegmentInfo.
            _msm_elem_to_info.emplace(msm_new_elem, msinfo);

            // Add this mortar segment to the secondary elem to mortar segment map
            secondary_to_msm_element_set.insert(msm_new_elem);

            _secondary_ip_sub_ids.insert(msinfo.secondary_elem->interior_parent()->subdomain_id());
            // Unlike for 2D, we always have a primary when building the mortar mesh so we don't
            // have to check for null
            _primary_ip_sub_ids.insert(msinfo.primary_elem->interior_parent()->subdomain_id());
          }
        }
        // End loop through primary element candidates
      }

      for (auto sel : make_range(secondary_side_elem->n_sub_elem()))
      {
        // Check if any segments failed to project
        if (mortar_segment_helper[sel]->remainder() == 1.0)
          mooseDoOnce(
              mooseWarning("Some secondary elements on mortar interface were unable to identify"
                           " a corresponding primary element; this may be expected depending on"
                           " problem geometry but may indicate a failure of the element search"
                           " or projection"));
      }

      if (secondary_to_msm_element_set.empty())
        _secondary_elems_to_mortar_segments.erase(secondary_elem_to_msm_map_it);
    } // End loop through secondary elements
  }   // End loop through mortar constraint pairs

  _mortar_segment_mesh->cache_elem_data();

  // Output mortar segment mesh
  if (_debug)
  {
    // If element is not triangular, increment subdomain id
    // (ExodusII does not support mixed element types in a single subdomain)
    for (const auto msm_el : _mortar_segment_mesh->active_local_element_ptr_range())
      if (msm_el->type() != TRI3)
        msm_el->subdomain_id()++;

    ExodusII_IO mortar_segment_mesh_writer(*_mortar_segment_mesh);

    // Default to non-HDF5 output for wider compatibility
    mortar_segment_mesh_writer.set_hdf5_writing(false);

    mortar_segment_mesh_writer.write("mortar_segment_mesh.e");

    // Undo increment
    for (const auto msm_el : _mortar_segment_mesh->active_local_element_ptr_range())
      if (msm_el->type() != TRI3)
        msm_el->subdomain_id()--;
  }

  buildCouplingInformation();

  // Print mortar segment mesh statistics
  if (_debug)
  {
    if (_mesh.n_processors() == 1)
      msmStatistics();
    else
      mooseWarning("Mortar segment mesh statistics intended for debugging purposes in serial only, "
                   "parallel will only provide statistics for local mortar segment mesh.");
  }
}

void
AutomaticMortarGeneration::buildCouplingInformation()
{
  std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, dof_id_type>>>
      coupling_info;

  // Loop over the msm_elem_to_info object and build a bi-directional
  // multimap from secondary elements to the primary Elems which they are
  // coupled to and vice-versa. This is used in the
  // AugmentSparsityOnInterface functor to determine whether a given
  // secondary Elem is coupled across the mortar interface to a primary
  // element.
  for (const auto & pr : _msm_elem_to_info)
  {
    const Elem * secondary_elem = pr.second.secondary_elem;
    const Elem * primary_elem = pr.second.primary_elem;

    // LowerSecondary
    coupling_info[secondary_elem->processor_id()].emplace_back(
        secondary_elem->id(), secondary_elem->interior_parent()->id());
    if (secondary_elem->processor_id() != _mesh.processor_id())
      // We want to keep information for nonlocal lower-dimensional secondary element point
      // neighbors for mortar nodal aux kernels
      _mortar_interface_coupling[secondary_elem->id()].insert(
          secondary_elem->interior_parent()->id());

    // LowerPrimary
    coupling_info[secondary_elem->processor_id()].emplace_back(
        secondary_elem->id(), primary_elem->interior_parent()->id());
    if (secondary_elem->processor_id() != _mesh.processor_id())
      // We want to keep information for nonlocal lower-dimensional secondary element point
      // neighbors for mortar nodal aux kernels
      _mortar_interface_coupling[secondary_elem->id()].insert(
          primary_elem->interior_parent()->id());

    // Lower-LowerDimensionalPrimary
    coupling_info[secondary_elem->processor_id()].emplace_back(secondary_elem->id(),
                                                               primary_elem->id());
    if (secondary_elem->processor_id() != _mesh.processor_id())
      // We want to keep information for nonlocal lower-dimensional secondary element point
      // neighbors for mortar nodal aux kernels
      _mortar_interface_coupling[secondary_elem->id()].insert(primary_elem->id());

    // SecondaryLower
    coupling_info[secondary_elem->interior_parent()->processor_id()].emplace_back(
        secondary_elem->interior_parent()->id(), secondary_elem->id());

    // SecondaryPrimary
    coupling_info[secondary_elem->interior_parent()->processor_id()].emplace_back(
        secondary_elem->interior_parent()->id(), primary_elem->interior_parent()->id());

    // PrimaryLower
    coupling_info[primary_elem->interior_parent()->processor_id()].emplace_back(
        primary_elem->interior_parent()->id(), secondary_elem->id());

    // PrimarySecondary
    coupling_info[primary_elem->interior_parent()->processor_id()].emplace_back(
        primary_elem->interior_parent()->id(), secondary_elem->interior_parent()->id());
  }

  // Push the coupling information
  auto action_functor =
      [this](processor_id_type,
             const std::vector<std::pair<dof_id_type, dof_id_type>> & coupling_info)
  {
    for (auto [i, j] : coupling_info)
      _mortar_interface_coupling[i].insert(j);
  };
  TIMPI::push_parallel_vector_data(_mesh.comm(), coupling_info, action_functor);
}

void
AutomaticMortarGeneration::msmStatistics()
{
  // Print boundary pairs
  Moose::out << "Mortar Interface Statistics:" << std::endl;

  // Count number of elements on primary and secondary sides
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
  {
    const auto primary_subd_id = pr.first;
    const auto secondary_subd_id = pr.second;

    // Allocate statistics vectors for primary lower, secondary lower, and msm meshes
    StatisticsVector<Real> primary;   // primary.reserve(mesh.n_elem());
    StatisticsVector<Real> secondary; // secondary.reserve(mesh.n_elem());
    StatisticsVector<Real> msm;       // msm.reserve(mortar_segment_mesh->n_elem());

    for (auto * el : _mesh.active_element_ptr_range())
    {
      // Add secondary and primary elem volumes to statistics vector
      if (el->subdomain_id() == secondary_subd_id)
        secondary.push_back(el->volume());
      else if (el->subdomain_id() == primary_subd_id)
        primary.push_back(el->volume());
    }

    // Note: when we allow more than one primary secondary pair will need to make
    // separate mortar segment mesh for each
    for (auto msm_elem : _mortar_segment_mesh->active_local_element_ptr_range())
    {
      // Add msm elem volume to statistic vector
      msm.push_back(msm_elem->volume());
    }

    // Create table
    std::vector<std::string> col_names = {"mesh", "n_elems", "max", "min", "median"};
    std::vector<std::string> subds = {"secondary_lower", "primary_lower", "mortar_segment"};
    std::vector<size_t> n_elems = {secondary.size(), primary.size(), msm.size()};
    std::vector<Real> maxs = {secondary.maximum(), primary.maximum(), msm.maximum()};
    std::vector<Real> mins = {secondary.minimum(), primary.minimum(), msm.minimum()};
    std::vector<Real> medians = {secondary.median(), primary.median(), msm.median()};

    FormattedTable table;
    table.clear();
    for (auto i : index_range(subds))
    {
      table.addRow(i);
      table.addData<std::string>(col_names[0], subds[i]);
      table.addData<size_t>(col_names[1], n_elems[i]);
      table.addData<Real>(col_names[2], maxs[i]);
      table.addData<Real>(col_names[3], mins[i]);
      table.addData<Real>(col_names[4], medians[i]);
    }

    Moose::out << "secondary subdomain: " << secondary_subd_id
               << " \tprimary subdomain: " << primary_subd_id << std::endl;
    table.printTable(Moose::out, subds.size());
  }
}

// The blocks marked with **** are for regressing edge dropping treatment and should be removed
// eventually.
//****
// Compute inactve nodes when the old (incorrect) edge dropping treatemnt is enabled
void
AutomaticMortarGeneration::computeIncorrectEdgeDroppingInactiveLMNodes()
{
  // Note that in 3D our trick to check whether an element has edge dropping needs loose tolerances
  // since the mortar segments are on the linearized element and comparing the volume of the
  // linearized element does not have the same volume as the warped element
  const Real tol = (dim() == 3) ? 0.1 : TOLERANCE;

  std::unordered_map<processor_id_type, std::set<dof_id_type>> proc_to_inactive_nodes_set;
  const auto my_pid = _mesh.processor_id();

  // List of inactive nodes on local secondary elements
  std::unordered_set<dof_id_type> inactive_node_ids;

  std::unordered_map<const Elem *, Real> active_volume{};

  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
    for (const auto el : _mesh.active_subdomain_elements_ptr_range(pr.second))
      active_volume[el] = 0.;

  // Compute fraction of elements with corresponding primary elements
  for (const auto msm_elem : _mortar_segment_mesh->active_local_element_ptr_range())
  {
    const MortarSegmentInfo & msinfo = _msm_elem_to_info.at(msm_elem);
    const Elem * secondary_elem = msinfo.secondary_elem;

    active_volume[secondary_elem] += msm_elem->volume();
  }

  // Mark all inactive local nodes
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
    // Loop through all elements on my processor
    for (const auto el : _mesh.active_local_subdomain_elements_ptr_range(pr.second))
      // If elem fully or partially dropped
      if (std::abs(active_volume[el] / el->volume() - 1.0) > tol)
      {
        // Add all nodes to list of inactive
        for (auto n : make_range(el->n_nodes()))
          inactive_node_ids.insert(el->node_id(n));
      }

  // Assemble list of procs that nodes contribute to
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
  {
    const auto secondary_subd_id = pr.second;

    // Loop through all elements not on my processor
    for (const auto el : _mesh.active_subdomain_elements_ptr_range(secondary_subd_id))
    {
      // Get processor_id
      const auto pid = el->processor_id();

      // If element is in my subdomain, skip
      if (pid == my_pid)
        continue;

      // If element on proc pid shares any of my inactive nodes, mark to send
      for (const auto n : make_range(el->n_nodes()))
      {
        const auto node_id = el->node_id(n);
        if (inactive_node_ids.find(node_id) != inactive_node_ids.end())
          proc_to_inactive_nodes_set[pid].insert(node_id);
      }
    }
  }

  // Send list of inactive nodes
  {
    // Pack set into vector for sending (push_parallel_vector_data doesn't like sets)
    std::unordered_map<processor_id_type, std::vector<dof_id_type>> proc_to_inactive_nodes_vector;
    for (const auto & proc_set : proc_to_inactive_nodes_set)
      proc_to_inactive_nodes_vector[proc_set.first].insert(
          proc_to_inactive_nodes_vector[proc_set.first].end(),
          proc_set.second.begin(),
          proc_set.second.end());

    // First push data
    auto action_functor = [this, &inactive_node_ids](const processor_id_type pid,
                                                     const std::vector<dof_id_type> & sent_data)
    {
      if (pid == _mesh.processor_id())
        mooseError("Should not be communicating with self.");
      for (const auto pr : sent_data)
        inactive_node_ids.insert(pr);
    };
    TIMPI::push_parallel_vector_data(_mesh.comm(), proc_to_inactive_nodes_vector, action_functor);
  }
  _inactive_local_lm_nodes.clear();
  for (const auto node_id : inactive_node_ids)
    _inactive_local_lm_nodes.insert(_mesh.node_ptr(node_id));
}

void
AutomaticMortarGeneration::computeInactiveLMNodes()
{
  if (!_correct_edge_dropping)
  {
    computeIncorrectEdgeDroppingInactiveLMNodes();
    return;
  }

  std::unordered_map<processor_id_type, std::set<dof_id_type>> proc_to_active_nodes_set;
  const auto my_pid = _mesh.processor_id();

  // List of active nodes on local secondary elements
  std::unordered_set<dof_id_type> active_local_nodes;

  // Mark all active local nodes
  for (const auto msm_elem : _mortar_segment_mesh->active_local_element_ptr_range())
  {
    const MortarSegmentInfo & msinfo = _msm_elem_to_info.at(msm_elem);
    const Elem * secondary_elem = msinfo.secondary_elem;

    for (auto n : make_range(secondary_elem->n_nodes()))
      active_local_nodes.insert(secondary_elem->node_id(n));
  }

  // Assemble list of procs that nodes contribute to
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
  {
    const auto secondary_subd_id = pr.second;

    // Loop through all elements not on my processor
    for (const auto el : _mesh.active_subdomain_elements_ptr_range(secondary_subd_id))
    {
      // Get processor_id
      const auto pid = el->processor_id();

      // If element is in my subdomain, skip
      if (pid == my_pid)
        continue;

      // If element on proc pid shares any of my active nodes, mark to send
      for (const auto n : make_range(el->n_nodes()))
      {
        const auto node_id = el->node_id(n);
        if (active_local_nodes.find(node_id) != active_local_nodes.end())
          proc_to_active_nodes_set[pid].insert(node_id);
      }
    }
  }

  // Send list of active nodes
  {
    // Pack set into vector for sending (push_parallel_vector_data doesn't like sets)
    std::unordered_map<processor_id_type, std::vector<dof_id_type>> proc_to_active_nodes_vector;
    for (const auto & proc_set : proc_to_active_nodes_set)
    {
      proc_to_active_nodes_vector[proc_set.first].reserve(proc_to_active_nodes_set.size());
      for (const auto node_id : proc_set.second)
        proc_to_active_nodes_vector[proc_set.first].push_back(node_id);
    }

    // First push data
    auto action_functor = [this, &active_local_nodes](const processor_id_type pid,
                                                      const std::vector<dof_id_type> & sent_data)
    {
      if (pid == _mesh.processor_id())
        mooseError("Should not be communicating with self.");
      active_local_nodes.insert(sent_data.begin(), sent_data.end());
    };
    TIMPI::push_parallel_vector_data(_mesh.comm(), proc_to_active_nodes_vector, action_functor);
  }

  // Every proc has correct list of active local nodes, now take complement (list of inactive nodes)
  // and store to use later to zero LM DoFs on inactive nodes
  _inactive_local_lm_nodes.clear();
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
    for (const auto el : _mesh.active_local_subdomain_elements_ptr_range(
             /*secondary_subd_id*/ pr.second))
      for (const auto n : make_range(el->n_nodes()))
        if (active_local_nodes.find(el->node_id(n)) == active_local_nodes.end())
          _inactive_local_lm_nodes.insert(el->node_ptr(n));
}

// Note: could be combined with previous routine, keeping separate for clarity (for now)
void
AutomaticMortarGeneration::computeInactiveLMElems()
{
  // Mark all active secondary elements
  std::unordered_set<const Elem *> active_local_elems;

  //****
  // Note that in 3D our trick to check whether an element has edge dropping needs loose tolerances
  // since the mortar segments are on the linearized element and comparing the volume of the
  // linearized element does not have the same volume as the warped element
  const Real tol = (dim() == 3) ? 0.1 : TOLERANCE;

  std::unordered_map<const Elem *, Real> active_volume;

  // Compute fraction of elements with corresponding primary elements
  if (!_correct_edge_dropping)
    for (const auto msm_elem : _mortar_segment_mesh->active_local_element_ptr_range())
    {
      const MortarSegmentInfo & msinfo = _msm_elem_to_info.at(msm_elem);
      const Elem * secondary_elem = msinfo.secondary_elem;

      active_volume[secondary_elem] += msm_elem->volume();
    }
  //****

  for (const auto msm_elem : _mortar_segment_mesh->active_local_element_ptr_range())
  {
    const MortarSegmentInfo & msinfo = _msm_elem_to_info.at(msm_elem);
    const Elem * secondary_elem = msinfo.secondary_elem;

    //****
    if (!_correct_edge_dropping)
      if (std::abs(active_volume[secondary_elem] / secondary_elem->volume() - 1.0) > tol)
        continue;
    //****

    active_local_elems.insert(secondary_elem);
  }

  // Take complement of active elements in active local subdomain to get inactive local elements
  _inactive_local_lm_elems.clear();
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
    for (const auto el : _mesh.active_local_subdomain_elements_ptr_range(
             /*secondary_subd_id*/ pr.second))
      if (active_local_elems.find(el) == active_local_elems.end())
        _inactive_local_lm_elems.insert(el);
}

void
AutomaticMortarGeneration::computeNodalGeometry()
{
  // The dimension according to Mesh::mesh_dimension().
  const auto dim = _mesh.mesh_dimension();

  // A nodal lower-dimensional nodal quadrature rule to be used on faces.
  QNodal qface(dim - 1);

  // A map from the node id to the attached elemental normals/weights evaluated at the node. Th
  // length of the vector will correspond to the number of elements attached to the node. If it is a
  // vertex node, for a 1D mortar mesh, the vector length will be two. If it is an interior node,
  // the vector will be length 1. The first member of the pair is that element's normal at the node.
  // The second member is that element's JxW at the node
  std::map<dof_id_type, std::vector<std::pair<Point, Real>>> node_to_normals_map;

  /// The _periodic flag tells us whether we want to inward vs outward facing normals
  Real sign = _periodic ? -1 : 1;

  // First loop over lower-dimensional secondary side elements and compute/save the outward normal
  // for each one. We loop over all active elements currently, but this procedure could be
  // parallelized as well.
  for (MeshBase::const_element_iterator el = _mesh.active_elements_begin(),
                                        end_el = _mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * secondary_elem = *el;

    // If this is not one of the lower-dimensional secondary side elements, go on to the next one.
    if (!_secondary_boundary_subdomain_ids.count(secondary_elem->subdomain_id()))
      continue;

    // We will create an FE object and attach the nodal quadrature rule such that we can get out the
    // normals at the element nodes
    FEType nnx_fe_type(secondary_elem->default_order(), LAGRANGE);
    std::unique_ptr<FEBase> nnx_fe_face(FEBase::build(dim, nnx_fe_type));
    nnx_fe_face->attach_quadrature_rule(&qface);
    const std::vector<Point> & face_normals = nnx_fe_face->get_normals();

    const auto & JxW = nnx_fe_face->get_JxW();

    // Which side of the parent are we? We need to know this to know
    // which side to reinit.
    const Elem * interior_parent = secondary_elem->interior_parent();
    mooseAssert(interior_parent,
                "No interior parent exists for element "
                    << secondary_elem->id()
                    << ". There may be a problem with your sideset set-up.");

    // Map to get lower dimensional element from interior parent on secondary surface
    // This map can be used to provide a handle to methods in this class that need to
    // operate on lower dimensional elements.
    _secondary_element_to_secondary_lowerd_element.emplace(interior_parent->id(), secondary_elem);

    // Look up which side of the interior parent secondary_elem is.
    auto s = interior_parent->which_side_am_i(secondary_elem);

    // Reinit the face FE object on side s.
    nnx_fe_face->reinit(interior_parent, s);

    for (MooseIndex(secondary_elem->n_nodes()) n = 0; n < secondary_elem->n_nodes(); ++n)
    {
      auto & normals_and_weights_vec = node_to_normals_map[secondary_elem->node_id(n)];
      normals_and_weights_vec.push_back(std::make_pair(sign * face_normals[n], JxW[n]));
    }
  }

  // Note that contrary to the Bin Yang dissertation, we are not weighting by the face element
  // lengths/volumes. It's not clear to me that this type of weighting is a good algorithm for cases
  // where the face can be curved
  for (const auto & pr : node_to_normals_map)
  {
    // Compute normal vector
    const auto & node_id = pr.first;
    const auto & normals_and_weights_vec = pr.second;

    Point nodal_normal;
    for (const auto & norm_and_weight : normals_and_weights_vec)
      nodal_normal += norm_and_weight.first * norm_and_weight.second;
    nodal_normal = nodal_normal.unit();

    _secondary_node_to_nodal_normal[_mesh.node_ptr(node_id)] = nodal_normal;

    Point nodal_tangent_one;
    Point nodal_tangent_two;
    householderOrthogolization(nodal_normal, nodal_tangent_one, nodal_tangent_two);

    _secondary_node_to_hh_nodal_tangents[_mesh.node_ptr(node_id)][0] = nodal_tangent_one;
    _secondary_node_to_hh_nodal_tangents[_mesh.node_ptr(node_id)][1] = nodal_tangent_two;
  }
}

void
AutomaticMortarGeneration::householderOrthogolization(const Point & nodal_normal,
                                                      Point & nodal_tangent_one,
                                                      Point & nodal_tangent_two) const
{
  mooseAssert(MooseUtils::absoluteFuzzyEqual(nodal_normal.norm(), 1),
              "The input nodal normal should have unity norm");

  const Real nx = nodal_normal(0);
  const Real ny = nodal_normal(1);
  const Real nz = nodal_normal(2);

  // See Lopes DS, Silva MT, Ambrosio JA. Tangent vectors to a 3-D surface normal: A geometric tool
  // to find orthogonal vectors based on the Householder transformation. Computer-Aided Design. 2013
  // Mar 1;45(3):683-94. We choose one definition of h_vector and deal with special case.
  const Point h_vector(nx + 1.0, ny, nz);

  // Avoid singularity of the equations at the end of routine by providing the solution to
  // (nx,ny,nz)=(-1,0,0) Normal/tangent fields can be visualized by outputting nodal geometry mesh
  // on a spherical problem.
  if (std::abs(h_vector(0)) < TOLERANCE)
  {
    nodal_tangent_one(0) = 0;
    nodal_tangent_one(1) = 1;
    nodal_tangent_one(2) = 0;

    nodal_tangent_two(0) = 0;
    nodal_tangent_two(1) = 0;
    nodal_tangent_two(2) = -1;

    return;
  }

  const Real h = h_vector.norm();

  nodal_tangent_one(0) = -2.0 * h_vector(0) * h_vector(1) / (h * h);
  nodal_tangent_one(1) = 1.0 - 2.0 * h_vector(1) * h_vector(1) / (h * h);
  nodal_tangent_one(2) = -2.0 * h_vector(1) * h_vector(2) / (h * h);

  nodal_tangent_two(0) = -2.0 * h_vector(0) * h_vector(2) / (h * h);
  nodal_tangent_two(1) = -2.0 * h_vector(1) * h_vector(2) / (h * h);
  nodal_tangent_two(2) = 1.0 - 2.0 * h_vector(2) * h_vector(2) / (h * h);
}

// Project secondary nodes onto their corresponding primary elements for each primary/secondary
// pair.
void
AutomaticMortarGeneration::projectSecondaryNodes()
{
  // For each primary/secondary boundary id pair, call the
  // project_secondary_nodes_single_pair() helper function.
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
    projectSecondaryNodesSinglePair(pr.first, pr.second);
}

void
AutomaticMortarGeneration::projectSecondaryNodesSinglePair(
    SubdomainID lower_dimensional_primary_subdomain_id,
    SubdomainID lower_dimensional_secondary_subdomain_id)
{
  // Build the "subdomain" adaptor based KD Tree.
  NanoflannMeshSubdomainAdaptor<3> mesh_adaptor(_mesh, lower_dimensional_primary_subdomain_id);
  subdomain_kd_tree_t kd_tree(
      3, mesh_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(/*max leaf=*/10));

  // Construct the KD tree.
  kd_tree.buildIndex();

  for (MeshBase::const_element_iterator el = _mesh.active_elements_begin(),
                                        end_el = _mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * secondary_side_elem = *el;

    // If this Elem is not in the current secondary subdomain, go on to the next one.
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
          this->_nodes_to_secondary_elem_map.at(secondary_node->id());

      // Check whether we've already mapped this secondary node
      // successfully for all of its nodal neighbors.
      bool is_mapped = true;
      for (MooseIndex(secondary_node_neighbors) snn = 0; snn < secondary_node_neighbors.size();
           ++snn)
      {
        auto secondary_key = std::make_pair(secondary_node, secondary_node_neighbors[snn]);
        if (!_secondary_node_and_elem_to_xi2_primary_elem.count(secondary_key))
        {
          is_mapped = false;
          break;
        }
      }

      // Go to the next node if this one has already been mapped.
      if (is_mapped)
        continue;

      // Look up the new nodal normal value in the local storage, error if not found.
      Point nodal_normal = _secondary_node_to_nodal_normal.at(secondary_node);

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
      kd_tree.findNeighbors(result_set, &query_pt[0], nanoflann::SearchParameters());

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
        // Verify that the squared distance we compute is the same as nanoflann'sFss
        mooseAssert(std::abs((_mesh.point(ret_index[r]) - *secondary_node).norm_sq() -
                             out_dist_sqr[r]) <= TOLERANCE,
                    "Lower-dimensional element squared distance verification failed.");

        // Get a reference to the vector of lower dimensional elements from the
        // nodes_to_primary_elem_map.
        std::vector<const Elem *> & primary_elem_candidates =
            this->_nodes_to_primary_elem_map.at(static_cast<dof_id_type>(ret_index[r]));

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
            const auto u = x2 - (*secondary_node);
            const auto F = u(0) * nodal_normal(1) - u(1) * nodal_normal(0);

            if (std::abs(F) < _newton_tolerance)
              break;

            if (F.derivatives())
            {
              Real dxi2 = -F.value() / F.derivatives();

              xi2_dn += dxi2;
            }
            else
              // It's possible that the secondary surface nodal normal is completely orthogonal to
              // the primary surface normal, in which case the derivative is 0. We know in this case
              // that the projection should be a failure
              current_iterate = max_iterates;
          } while (++current_iterate < max_iterates);

          Real xi2 = xi2_dn.value();

          // Check whether the projection worked. The last condition checks for obliqueness of the
          // projection
          if ((current_iterate < max_iterates) && (std::abs(xi2) <= 1. + _xi_tolerance) &&
              (std::abs(
                   (primary_elem_candidate->point(0) - primary_elem_candidate->point(1)).unit() *
                   nodal_normal) < std::cos(_minimum_projection_angle * libMesh::pi / 180)))
          {
            // If xi2 == +1 or -1 then this secondary node mapped directly to a node on the primary
            // surface. This isn't as unlikely as you might think, it will happen if the meshes
            // on the interface start off being perfectly aligned. In this situation, we need to
            // associate the secondary node with two different elements (and two corresponding
            // xi^(2) values.

            // We are projecting on one side first and the other side second. If we make the
            // tolerance bigger and remove the (5) factor we are going to continue to miss the
            // second projection and fall into the exception message in
            // projectPrimaryNodesSinglePair. What makes this modification to not fall in the
            // exception is that we are projecting on one side more xi than in the other. There
            // should be a better way of doing this by using actual distances and not parametric
            // coordinates. But I believe making the tolerance uniformly larger or smaller won't do
            // the trick here.
            if (std::abs(std::abs(xi2) - 1.) < _xi_tolerance * 5.0)
            {
              const Node * primary_node = (xi2 < 0) ? primary_elem_candidate->node_ptr(0)
                                                    : primary_elem_candidate->node_ptr(1);

              const std::vector<const Elem *> & primary_node_neighbors =
                  _nodes_to_primary_elem_map.at(primary_node->id());

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
                    _secondary_node_and_elem_to_xi2_primary_elem.emplace(secondary_key,
                                                                         primary_val);

                    // Also map in the other direction.
                    Real xi1 =
                        (secondary_node == secondary_node_neighbors[snn]->node_ptr(0)) ? -1 : +1;

                    auto primary_key = std::make_tuple(
                        primary_node->id(), primary_node, primary_node_neighbors[mnn]);
                    auto secondary_val = std::make_pair(xi1, secondary_node_neighbors[snn]);
                    _primary_node_and_elem_to_xi1_secondary_elem.emplace(primary_key,
                                                                         secondary_val);
                  }

              if (!found_match)
              {
                // There could be coincident nodes and this might be a bad primary candidate (see
                // issue #21680). Instead of giving up, let's try continuing
                rejected_primary_elem_candidates.insert(primary_elem_candidate);
                continue;
              }

              // We need to handle the case where we've exactly projected a secondary node onto a
              // primary node, but our secondary node is at one of the secondary face endpoints and
              // our primary node is not.
              if (secondary_node_neighbors.size() == 1 && primary_node_neighbors.size() == 2)
                for (auto it = primary_elems_mapped.begin(); it != primary_elems_mapped.end(); ++it)
                  if (*it == false)
                  {
                    auto index = std::distance(primary_elems_mapped.begin(), it);
                    _primary_node_and_elem_to_xi1_secondary_elem.emplace(
                        std::make_tuple(
                            primary_node->id(), primary_node, primary_node_neighbors[index]),
                        std::make_pair(1, nullptr));
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
                    _secondary_node_and_elem_to_xi2_primary_elem.emplace(key, val);
                  }
                }
              }
            }

            projection_succeeded = true;
            break; // out of e-loop
          }
          else
            // The current secondary_node is not in this Elem, so keep track of the rejects.
            rejected_primary_elem_candidates.insert(primary_elem_candidate);
        }

        if (projection_succeeded)
          break; // out of r-loop
      }          // r-loop

      if (!projection_succeeded && _debug)
        _console << "Failed to find primary Elem into which secondary node "
                 << static_cast<const Point &>(*secondary_node) << " was projected." << std::endl
                 << std::endl;
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
  for (const auto & pr : _primary_secondary_subdomain_id_pairs)
    projectPrimaryNodesSinglePair(pr.first, pr.second);
}

void
AutomaticMortarGeneration::projectPrimaryNodesSinglePair(
    SubdomainID lower_dimensional_primary_subdomain_id,
    SubdomainID lower_dimensional_secondary_subdomain_id)
{
  // Build a Nanoflann object on the lower-dimensional secondary elements of the Mesh.
  NanoflannMeshSubdomainAdaptor<3> mesh_adaptor(_mesh, lower_dimensional_secondary_subdomain_id);
  subdomain_kd_tree_t kd_tree(
      3, mesh_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(/*max leaf=*/10));

  // Construct the KD tree for lower-dimensional elements in the volume mesh.
  kd_tree.buildIndex();

  std::unordered_set<dof_id_type> primary_nodes_visited;

  for (const auto & primary_side_elem : _mesh.active_element_ptr_range())
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
          _nodes_to_primary_elem_map.at(primary_node->id());

      // Check whether we have already successfully inverse mapped this primary node (whether during
      // secondary node projection or now during primary node projection) or we have already failed
      // to inverse map this primary node (now during primary node projection), and then skip if
      // either of those things is true
      auto primary_key =
          std::make_tuple(primary_node->id(), primary_node, primary_node_neighbors[0]);
      if (!primary_nodes_visited.insert(primary_node->id()).second ||
          _primary_node_and_elem_to_xi1_secondary_elem.count(primary_key))
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
      kd_tree.findNeighbors(result_set, &query_pt[0], nanoflann::SearchParameters());

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
        mooseAssert(std::abs((_mesh.point(ret_index[r]) - *primary_node).norm_sq() -
                             out_dist_sqr[r]) <= TOLERANCE,
                    "Lower-dimensional element squared distance verification failed.");

        // Get a reference to the vector of lower dimensional elements from the
        // nodes_to_secondary_elem_map.
        const std::vector<const Elem *> & secondary_elem_candidates =
            _nodes_to_secondary_elem_map.at(static_cast<dof_id_type>(ret_index[r]));

        // Print the Elems connected to this node on the secondary mesh side.
        for (MooseIndex(secondary_elem_candidates) e = 0; e < secondary_elem_candidates.size(); ++e)
        {
          const Elem * secondary_elem_candidate = secondary_elem_candidates[e];

          // If we've already rejected this candidate, we don't need to check it again.
          if (rejected_secondary_elem_candidates.count(secondary_elem_candidate))
            continue;

          std::vector<Point> nodal_normals(secondary_elem_candidate->n_nodes());
          for (const auto n : make_range(secondary_elem_candidate->n_nodes()))
            nodal_normals[n] =
                _secondary_node_to_nodal_normal.at(secondary_elem_candidate->node_ptr(n));

          // Use equation 2.4.6 from Bin Yang's dissertation to try and solve for
          // the position on the secondary element where this primary came from.  This
          // requires a Newton iteration in general.
          DualNumber<Real> xi1_dn{0, 1}; // initial guess
          auto && order = secondary_elem_candidate->default_order();
          unsigned int current_iterate = 0, max_iterates = 10;

          VectorValue<DualNumber<Real>> normals(0);

          // Newton iteration loop - this to converge in 1 iteration when it
          // succeeds, and possibly two iterations when it converges to a
          // xi outside the reference element. I don't know any reason why it should
          // only take 1 iteration -- the Jacobian is not constant in general...
          do
          {
            VectorValue<DualNumber<Real>> x1(0);
            for (MooseIndex(secondary_elem_candidate->n_nodes()) n = 0;
                 n < secondary_elem_candidate->n_nodes();
                 ++n)
            {
              const auto phi = Moose::fe_lagrange_1D_shape(order, n, xi1_dn);
              x1 += phi * secondary_elem_candidate->point(n);
              normals += phi * nodal_normals[n];
            }

            const auto u = x1 - (*primary_node);

            const auto F = u(0) * normals(1) - u(1) * normals(0);

            if (std::abs(F) < _newton_tolerance)
              break;

            // Unlike for projection of nodal normals onto primary surfaces, we should never have a
            // case where the nodal normal is completely orthogonal to the secondary surface, so we
            // do not have to guard against F.derivatives() == 0 here
            Real dxi1 = -F.value() / F.derivatives();

            xi1_dn += dxi1;

            normals = 0;
          } while (++current_iterate < max_iterates);

          Real xi1 = xi1_dn.value();

          // Check for convergence to a valid solution... The last condition checks for obliqueness
          // of the projection
          if ((current_iterate < max_iterates) && (std::abs(xi1) <= 1. + _xi_tolerance) &&
              (std::abs((primary_side_elem->point(0) - primary_side_elem->point(1)).unit() *
                        MetaPhysicL::raw_value(normals).unit()) <
               std::cos(_minimum_projection_angle * libMesh::pi / 180.0)))
          {
            if (std::abs(std::abs(xi1) - 1.) < _xi_tolerance)
            {
              // Special case: xi1=+/-1.
              // It is unlikely that we get here, because this primary node should already
              // have been mapped during the project_secondary_nodes() routine, but
              // there is still a chance since the tolerances are applied to
              // the xi coordinate and that value may be different on a primary element and a
              // secondary element since they may have different sizes.
              throw MooseException("Nodes on primary and secondary surfaces are aligned. This is "
                                   "causing trouble when identifying projections from secondary "
                                   "nodes when performing primary node projections.");
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
                  _primary_node_and_elem_to_xi1_secondary_elem.emplace(key, val);
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

std::vector<AutomaticMortarGeneration::MortarFilterIter>
AutomaticMortarGeneration::secondariesToMortarSegments(const Node & node) const
{
  auto secondary_it = _nodes_to_secondary_elem_map.find(node.id());
  if (secondary_it == _nodes_to_secondary_elem_map.end())
    return {};

  const auto & secondary_elems = secondary_it->second;
  std::vector<MortarFilterIter> ret;
  ret.reserve(secondary_elems.size());

  for (const auto i : index_range(secondary_elems))
  {
    auto * const secondary_elem = secondary_elems[i];
    auto msm_it = _secondary_elems_to_mortar_segments.find(secondary_elem->id());
    if (msm_it == _secondary_elems_to_mortar_segments.end())
      // We may have removed this element key from this map
      continue;

    mooseAssert(secondary_elem->active(),
                "We loop over active elements when building the mortar segment mesh, so we golly "
                "well hope this is active.");
    mooseAssert(!msm_it->second.empty(),
                "We should have removed all secondaries from this map if they do not have any "
                "mortar segments associated with them.");
    ret.push_back(msm_it);
  }

  return ret;
}
