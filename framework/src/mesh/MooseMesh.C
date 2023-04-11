//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseMesh.h"
#include "Factory.h"
#include "CacheChangedListsThread.h"
#include "MooseUtils.h"
#include "MooseApp.h"
#include "RelationshipManager.h"
#include "PointListAdaptor.h"
#include "Executioner.h"
#include "NonlinearSystemBase.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "MooseVariableBase.h"
#include "MooseMeshUtils.h"
#include "MooseAppCoordTransform.h"

#include <utility>

// libMesh
#include "libmesh/bounding_box.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/fe_base.h"
#include "libmesh/fe_interface.h"
#include "libmesh/mesh_inserter_iterator.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_inserter_iterator.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"
#include "libmesh/parallel_ghost_sync.h"
#include "libmesh/utility.h"
#include "libmesh/remote_elem.h"
#include "libmesh/linear_partitioner.h"
#include "libmesh/centroid_partitioner.h"
#include "libmesh/parmetis_partitioner.h"
#include "libmesh/hilbert_sfc_partitioner.h"
#include "libmesh/morton_sfc_partitioner.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/quadrature.h"
#include "libmesh/boundary_info.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/default_coupling.h"
#include "libmesh/ghost_point_neighbors.h"
#include "libmesh/fe_type.h"

static const int GRAIN_SIZE =
    1; // the grain_size does not have much influence on our execution speed

InputParameters
MooseMesh::validParams()
{
  InputParameters params = MooseObject::validParams();

  MooseEnum parallel_type("DEFAULT REPLICATED DISTRIBUTED", "DEFAULT");
  params.addParam<MooseEnum>("parallel_type",
                             parallel_type,
                             "DEFAULT: Use libMesh::ReplicatedMesh unless --distributed-mesh is "
                             "specified on the command line "
                             "REPLICATED: Always use libMesh::ReplicatedMesh "
                             "DISTRIBUTED: Always use libMesh::DistributedMesh");

  params.addParam<bool>(
      "allow_renumbering",
      true,
      "If allow_renumbering=false, node and element numbers are kept fixed until deletion");

  params.addParam<bool>("nemesis",
                        false,
                        "If nemesis=true and file=foo.e, actually reads "
                        "foo.e.N.0, foo.e.N.1, ... foo.e.N.N-1, "
                        "where N = # CPUs, with NemesisIO.");

  MooseEnum dims("1=1 2 3", "1");
  params.addParam<MooseEnum>("dim",
                             dims,
                             "This is only required for certain mesh formats where "
                             "the dimension of the mesh cannot be autodetected. "
                             "In particular you must supply this for GMSH meshes. "
                             "Note: This is completely ignored for ExodusII meshes!");

  params.addParam<MooseEnum>(
      "partitioner",
      partitioning(),
      "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation.");
  MooseEnum direction("x y z radial");
  params.addParam<MooseEnum>("centroid_partitioner_direction",
                             direction,
                             "Specifies the sort direction if using the centroid partitioner. "
                             "Available options: x, y, z, radial");

  MooseEnum patch_update_strategy("never always auto iteration", "never");
  params.addParam<MooseEnum>(
      "patch_update_strategy",
      patch_update_strategy,
      "How often to update the geometric search 'patch'.  The default is to "
      "never update it (which is the most efficient but could be a problem "
      "with lots of relative motion). 'always' will update the patch for all "
      "secondary nodes at the beginning of every timestep which might be time "
      "consuming. 'auto' will attempt to determine at the start of which "
      "timesteps the patch for all secondary nodes needs to be updated automatically."
      "'iteration' updates the patch at every nonlinear iteration for a "
      "subset of secondary nodes for which penetration is not detected. If there "
      "can be substantial relative motion between the primary and secondary surfaces "
      "during the nonlinear iterations within a timestep, it is advisable to use "
      "'iteration' option to ensure accurate contact detection.");

  // Note: This parameter is named to match 'construct_side_list_from_node_list' in SetupMeshAction
  params.addParam<bool>(
      "construct_node_list_from_side_list",
      true,
      "Whether or not to generate nodesets from the sidesets (usually a good idea).");
  params.addParam<unsigned int>(
      "patch_size", 40, "The number of nodes to consider in the NearestNode neighborhood.");
  params.addParam<unsigned int>("ghosting_patch_size",
                                "The number of nearest neighbors considered "
                                "for ghosting purposes when 'iteration' "
                                "patch update strategy is used. Default is "
                                "5 * patch_size.");
  params.addParam<unsigned int>("max_leaf_size",
                                10,
                                "The maximum number of points in each leaf of the KDTree used in "
                                "the nearest neighbor search. As the leaf size becomes larger,"
                                "KDTree construction becomes faster but the nearest neighbor search"
                                "becomes slower.");

  params.addParam<bool>("build_all_side_lowerd_mesh",
                        false,
                        "True to build the lower-dimensional mesh for all sides.");

  params.addParam<bool>("skip_refine_when_use_split",
                        true,
                        "True to skip uniform refinements when using a pre-split mesh.");

  params.addParam<std::vector<SubdomainID>>(
      "add_subdomain_ids",
      "The listed subdomains will be assumed valid for the mesh. This permits setting up subdomain "
      "restrictions for subdomains initially containing no elements, which can occur, for example, "
      "in additive manufacturing simulations which dynamically add and remove elements.");

  params += MooseAppCoordTransform::validParams();

  // This indicates that the derived mesh type accepts a MeshGenerator, and should be set to true in
  // derived types that do so.
  params.addPrivateParam<bool>("_mesh_generator_mesh", false);

  // Whether or not the mesh is pre split
  params.addPrivateParam<bool>("_is_split", false);

  params.registerBase("MooseMesh");

  // groups
  params.addParamNamesToGroup(
      "dim nemesis patch_update_strategy construct_node_list_from_side_list patch_size",
      "Advanced");
  params.addParamNamesToGroup("partitioner centroid_partitioner_direction", "Partitioning");

  return params;
}

MooseMesh::MooseMesh(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "Mesh"),
    PerfGraphInterface(this),
    _parallel_type(getParam<MooseEnum>("parallel_type").getEnum<MooseMesh::ParallelType>()),
    _use_distributed_mesh(false),
    _distribution_overridden(false),
    _parallel_type_overridden(false),
    _mesh(nullptr),
    _partitioner_name(getParam<MooseEnum>("partitioner")),
    _partitioner_overridden(false),
    _custom_partitioner_requested(false),
    _uniform_refine_level(0),
    _skip_refine_when_use_split(getParam<bool>("skip_refine_when_use_split")),
    _skip_deletion_repartition_after_refine(false),
    _is_nemesis(getParam<bool>("nemesis")),
    _node_to_elem_map_built(false),
    _node_to_active_semilocal_elem_map_built(false),
    _patch_size(getParam<unsigned int>("patch_size")),
    _ghosting_patch_size(isParamValid("ghosting_patch_size")
                             ? getParam<unsigned int>("ghosting_patch_size")
                             : 5 * _patch_size),
    _max_leaf_size(getParam<unsigned int>("max_leaf_size")),
    _patch_update_strategy(
        getParam<MooseEnum>("patch_update_strategy").getEnum<Moose::PatchUpdateType>()),
    _regular_orthogonal_mesh(false),
    _is_split(getParam<bool>("_is_split")),
    _allow_recovery(true),
    _construct_node_list_from_side_list(getParam<bool>("construct_node_list_from_side_list")),
    _need_delete(false),
    _allow_remote_element_removal(true),
    _need_ghost_ghosted_boundaries(true),
    _is_displaced(false),
    _rz_coord_axis(getParam<MooseEnum>("rz_coord_axis")),
    _coord_system_set(false)
{
  if (isParamValid("ghosting_patch_size") && (_patch_update_strategy != Moose::Iteration))
    mooseError("Ghosting patch size parameter has to be set in the mesh block "
               "only when 'iteration' patch update strategy is used.");

  if (isParamValid("coord_block"))
  {
    if (isParamValid("block"))
      paramWarning("block",
                   "You set both 'Mesh/block' and 'Mesh/coord_block'. The value of "
                   "'Mesh/coord_block' will be used.");

    _provided_coord_blocks = getParam<std::vector<SubdomainName>>("coord_block");
  }
  else if (isParamValid("block"))
    _provided_coord_blocks = getParam<std::vector<SubdomainName>>("block");

  if (getParam<bool>("build_all_side_lowerd_mesh"))
    // Do not initially allow removal of remote elements
    allowRemoteElementRemoval(false);

  determineUseDistributedMesh();
}

MooseMesh::MooseMesh(const MooseMesh & other_mesh)
  : MooseObject(other_mesh._pars),
    Restartable(this, "Mesh"),
    PerfGraphInterface(this, "CopiedMesh"),
    _parallel_type(other_mesh._parallel_type),
    _use_distributed_mesh(other_mesh._use_distributed_mesh),
    _distribution_overridden(other_mesh._distribution_overridden),
    _parallel_type_overridden(other_mesh._parallel_type_overridden),
    _mesh(other_mesh.getMesh().clone()),
    _partitioner_name(other_mesh._partitioner_name),
    _partitioner_overridden(other_mesh._partitioner_overridden),
    _custom_partitioner_requested(other_mesh._custom_partitioner_requested),
    _uniform_refine_level(other_mesh.uniformRefineLevel()),
    _skip_refine_when_use_split(other_mesh._skip_refine_when_use_split),
    _skip_deletion_repartition_after_refine(other_mesh._skip_deletion_repartition_after_refine),
    _is_nemesis(false),
    _node_to_elem_map_built(false),
    _node_to_active_semilocal_elem_map_built(false),
    _patch_size(other_mesh._patch_size),
    _ghosting_patch_size(other_mesh._ghosting_patch_size),
    _max_leaf_size(other_mesh._max_leaf_size),
    _patch_update_strategy(other_mesh._patch_update_strategy),
    _regular_orthogonal_mesh(false),
    _is_split(other_mesh._is_split),
    _construct_node_list_from_side_list(other_mesh._construct_node_list_from_side_list),
    _need_delete(other_mesh._need_delete),
    _allow_remote_element_removal(other_mesh._allow_remote_element_removal),
    _need_ghost_ghosted_boundaries(other_mesh._need_ghost_ghosted_boundaries),
    _coord_sys(other_mesh._coord_sys),
    _rz_coord_axis(other_mesh._rz_coord_axis),
    _coord_system_set(other_mesh._coord_system_set),
    _provided_coord_blocks(other_mesh._provided_coord_blocks)
{
  // Note: this calls BoundaryInfo::operator= without changing the
  // ownership semantics of either Mesh's BoundaryInfo object.
  getMesh().get_boundary_info() = other_mesh.getMesh().get_boundary_info();

  const std::set<SubdomainID> & subdomains = other_mesh.meshSubdomains();
  for (const auto & sbd_id : subdomains)
    setSubdomainName(sbd_id, other_mesh.getMesh().subdomain_name(sbd_id));

  // Get references to BoundaryInfo objects to make the code below cleaner...
  const BoundaryInfo & other_boundary_info = other_mesh.getMesh().get_boundary_info();
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  // Use the other BoundaryInfo object to build the list of side boundary ids
  std::vector<BoundaryID> side_boundaries;
  other_boundary_info.build_side_boundary_ids(side_boundaries);

  // Assign those boundary ids in our BoundaryInfo object
  for (const auto & side_bnd_id : side_boundaries)
    boundary_info.sideset_name(side_bnd_id) = other_boundary_info.get_sideset_name(side_bnd_id);

  // Do the same thing for node boundary ids
  std::vector<BoundaryID> node_boundaries;
  other_boundary_info.build_node_boundary_ids(node_boundaries);

  for (const auto & node_bnd_id : node_boundaries)
    boundary_info.nodeset_name(node_bnd_id) = other_boundary_info.get_nodeset_name(node_bnd_id);

  _bounds.resize(other_mesh._bounds.size());
  for (std::size_t i = 0; i < _bounds.size(); ++i)
  {
    _bounds[i].resize(other_mesh._bounds[i].size());
    for (std::size_t j = 0; j < _bounds[i].size(); ++j)
      _bounds[i][j] = other_mesh._bounds[i][j];
  }

  updateCoordTransform();
}

MooseMesh::~MooseMesh()
{
  freeBndNodes();
  freeBndElems();
  clearQuadratureNodes();
}

void
MooseMesh::freeBndNodes()
{
  // free memory
  for (auto & bnode : _bnd_nodes)
    delete bnode;

  for (auto & it : _node_set_nodes)
    it.second.clear();

  _node_set_nodes.clear();

  for (auto & it : _bnd_node_ids)
    it.second.clear();

  _bnd_node_ids.clear();
}

void
MooseMesh::freeBndElems()
{
  // free memory
  for (auto & belem : _bnd_elems)
    delete belem;

  for (auto & it : _bnd_elem_ids)
    it.second.clear();

  _bnd_elem_ids.clear();
}

void
MooseMesh::prepare(bool)
{
  TIME_SECTION("prepare", 2, "Preparing Mesh", true);

  mooseAssert(_mesh, "The MeshBase has not been constructed");

  if (!dynamic_cast<DistributedMesh *>(&getMesh()) || _is_nemesis)
    // For whatever reason we do not want to allow renumbering here nor ever in the future?
    getMesh().allow_renumbering(false);

  if (!_mesh->is_prepared())
  {
    _mesh->prepare_for_use();

    _moose_mesh_prepared = false;
  }

  if (_moose_mesh_prepared)
    return;

  // Collect (local) subdomain IDs
  _mesh_subdomains.clear();
  for (const auto & elem : getMesh().element_ptr_range())
    _mesh_subdomains.insert(elem->subdomain_id());

  // add explicitly requested subdomains
  if (isParamValid("add_subdomain_ids"))
  {
    const auto add_subdomain_id = getParam<std::vector<SubdomainID>>("add_subdomain_ids");
    _mesh_subdomains.insert(add_subdomain_id.begin(), add_subdomain_id.end());
  }

  // Make sure nodesets have been generated
  buildNodeListFromSideList();

  // Collect (local) boundary IDs
  const std::set<BoundaryID> & local_bids = getMesh().get_boundary_info().get_boundary_ids();
  _mesh_boundary_ids.insert(local_bids.begin(), local_bids.end());

  const std::set<BoundaryID> & local_node_bids =
      getMesh().get_boundary_info().get_node_boundary_ids();
  _mesh_nodeset_ids.insert(local_node_bids.begin(), local_node_bids.end());

  const std::set<BoundaryID> & local_side_bids =
      getMesh().get_boundary_info().get_side_boundary_ids();
  _mesh_sideset_ids.insert(local_side_bids.begin(), local_side_bids.end());

  // Communicate subdomain and boundary IDs if this is a parallel mesh
  if (!getMesh().is_serial())
  {
    _communicator.set_union(_mesh_subdomains);
    _communicator.set_union(_mesh_boundary_ids);
    _communicator.set_union(_mesh_nodeset_ids);
    _communicator.set_union(_mesh_sideset_ids);
  }

  if (!_coord_system_set)
    setCoordSystem(_provided_coord_blocks, getParam<MultiMooseEnum>("coord_type"));
  else if (_pars.isParamSetByUser("coord_type"))
    mooseError(
        "Trying to set coordinate system type information based on the user input file, but "
        "the coordinate system type information has already been set programmatically! "
        "Either remove your coordinate system type information from the input file, or contact "
        "your application developer");

  detectOrthogonalDimRanges();

  update();

  // Check if there is subdomain name duplication for the same subdomain ID
  checkDuplicateSubdomainNames();

  _moose_mesh_prepared = true;
}

void
MooseMesh::update()
{
  TIME_SECTION("update", 3, "Updating Mesh", true);

  // Rebuild the boundary conditions
  buildNodeListFromSideList();

  // Update the node to elem map
  _node_to_elem_map.clear();
  _node_to_elem_map_built = false;
  _node_to_active_semilocal_elem_map.clear();
  _node_to_active_semilocal_elem_map_built = false;

  buildNodeList();
  buildBndElemList();
  cacheInfo();
  buildElemIDInfo();

  _finite_volume_info_dirty = true;
}

void
MooseMesh::buildLowerDMesh()
{
  auto & mesh = getMesh();

  if (!mesh.is_serial())
    mooseError(
        "Hybrid finite element method must use replicated mesh.\nCurrently lower-dimensional mesh "
        "does not support mesh re-partitioning and a debug assertion being hit related with "
        "neighbors of lower-dimensional element, with distributed mesh.");

  // Lower-D element build requires neighboring element information
  if (!mesh.is_prepared())
    mesh.find_neighbors();

  // maximum number of sides of all elements
  unsigned int max_n_sides = 0;

  // remove existing lower-d element first
  std::set<Elem *> deleteable_elems;
  for (auto & elem : mesh.element_ptr_range())
    if (elem->subdomain_id() == Moose::INTERNAL_SIDE_LOWERD_ID ||
        elem->subdomain_id() == Moose::BOUNDARY_SIDE_LOWERD_ID)
      deleteable_elems.insert(elem);
    else if (elem->n_sides() > max_n_sides)
      max_n_sides = elem->n_sides();

  for (auto & elem : deleteable_elems)
    mesh.delete_elem(elem);

  mesh.comm().max(max_n_sides);

  deleteable_elems.clear();

  dof_id_type max_elem_id = mesh.max_elem_id();
  unique_id_type max_unique_id = mesh.parallel_max_unique_id();

  std::vector<Elem *> side_elems;
  _higher_d_elem_side_to_lower_d_elem.clear();
  for (const auto & elem : mesh.active_element_ptr_range())
  {
    // skip existing lower-d elements
    if (elem->interior_parent())
      continue;

    for (const auto side : elem->side_index_range())
    {
      Elem * neig = elem->neighbor_ptr(side);

      bool build_side = false;
      if (!neig)
        build_side = true;
      else
      {
        mooseAssert(!neig->is_remote(), "We error if the mesh is not serial");
        if (!neig->active())
          build_side = true;
        else if (neig->level() == elem->level() && elem->id() < neig->id())
          build_side = true;
      }

      if (build_side)
      {
        std::unique_ptr<Elem> side_elem(elem->build_side_ptr(side, false));

        // The side will be added with the same processor id as the parent.
        side_elem->processor_id() = elem->processor_id();

        // Add subdomain ID
        if (neig)
          side_elem->subdomain_id() = Moose::INTERNAL_SIDE_LOWERD_ID;
        else
          side_elem->subdomain_id() = Moose::BOUNDARY_SIDE_LOWERD_ID;

        // set ids consistently across processors (these ids will be temporary)
        side_elem->set_id(max_elem_id + elem->id() * max_n_sides + side);
        side_elem->set_unique_id(max_unique_id + elem->id() * max_n_sides + side);

        // Also assign the side's interior parent, so it is always
        // easy to figure out the Elem we came from.
        // Note: the interior parent could be a ghost element.
        side_elem->set_interior_parent(elem);

        side_elems.push_back(side_elem.release());

        // add link between higher d element to lower d element
        auto pair = std::make_pair(elem, side);
        auto link = std::make_pair(pair, side_elems.back());
        auto ilink = std::make_pair(side_elems.back(), side);
        _lower_d_elem_to_higher_d_elem_side.insert(ilink);
        _higher_d_elem_side_to_lower_d_elem.insert(link);
      }
    }
  }

  // finally, add the lower-dimensional element to the mesh
  // Note: lower-d interior element will exist on a processor if its associated interior
  //       parent exists on a processor whether or not being a ghost. Lower-d elements will
  //       get its interior parent's processor id.
  for (auto & elem : side_elems)
    mesh.add_elem(elem);

  _mesh_subdomains.insert(Moose::INTERNAL_SIDE_LOWERD_ID);
  mesh.subdomain_name(Moose::INTERNAL_SIDE_LOWERD_ID) = "INTERNAL_SIDE_LOWERD_SUBDOMAIN";
  _mesh_subdomains.insert(Moose::BOUNDARY_SIDE_LOWERD_ID);
  mesh.subdomain_name(Moose::BOUNDARY_SIDE_LOWERD_ID) = "BOUNDARY_SIDE_LOWERD_SUBDOMAIN";

  // we do all the stuff in prepare_for_use such as renumber_nodes_and_elements(),
  // update_parallel_id_counts(), cache_elem_dims(), etc. except partitioning here.
  const bool skip_partitioning_old = mesh.skip_partitioning();
  mesh.skip_partitioning(true);
  mesh.prepare_for_use();
  mesh.skip_partitioning(skip_partitioning_old);
}

const Node &
MooseMesh::node(const dof_id_type i) const
{
  mooseDeprecated("MooseMesh::node() is deprecated, please use MooseMesh::nodeRef() instead");
  return nodeRef(i);
}

Node &
MooseMesh::node(const dof_id_type i)
{
  mooseDeprecated("MooseMesh::node() is deprecated, please use MooseMesh::nodeRef() instead");
  return nodeRef(i);
}

const Node &
MooseMesh::nodeRef(const dof_id_type i) const
{
  if (i > getMesh().max_node_id())
    return *(*_quadrature_nodes.find(i)).second;

  return getMesh().node_ref(i);
}

Node &
MooseMesh::nodeRef(const dof_id_type i)
{
  if (i > getMesh().max_node_id())
    return *_quadrature_nodes[i];

  return getMesh().node_ref(i);
}

const Node *
MooseMesh::nodePtr(const dof_id_type i) const
{
  if (i > getMesh().max_node_id())
    return (*_quadrature_nodes.find(i)).second;

  return getMesh().node_ptr(i);
}

Node *
MooseMesh::nodePtr(const dof_id_type i)
{
  if (i > getMesh().max_node_id())
    return _quadrature_nodes[i];

  return getMesh().node_ptr(i);
}

const Node *
MooseMesh::queryNodePtr(const dof_id_type i) const
{
  if (i > getMesh().max_node_id())
    return (*_quadrature_nodes.find(i)).second;

  return getMesh().query_node_ptr(i);
}

Node *
MooseMesh::queryNodePtr(const dof_id_type i)
{
  if (i > getMesh().max_node_id())
    return _quadrature_nodes[i];

  return getMesh().query_node_ptr(i);
}

void
MooseMesh::meshChanged()
{
  TIME_SECTION("meshChanged", 3, "Updating Because Mesh Changed");

  update();

  // Delete all of the cached ranges
  _active_local_elem_range.reset();
  _active_node_range.reset();
  _active_semilocal_node_range.reset();
  _local_node_range.reset();
  _bnd_node_range.reset();
  _bnd_elem_range.reset();

  // Rebuild the ranges
  getActiveLocalElementRange();
  getActiveNodeRange();
  getLocalNodeRange();
  getBoundaryNodeRange();
  getBoundaryElementRange();

  // Call the callback function onMeshChanged
  onMeshChanged();
}

void
MooseMesh::onMeshChanged()
{
}

void
MooseMesh::cacheChangedLists()
{
  TIME_SECTION("cacheChangedLists", 5, "Caching Changed Lists");

  ConstElemRange elem_range(getMesh().local_elements_begin(), getMesh().local_elements_end(), 1);
  CacheChangedListsThread cclt(*this);
  Threads::parallel_reduce(elem_range, cclt);

  _coarsened_element_children.clear();

  _refined_elements = std::make_unique<ConstElemPointerRange>(cclt._refined_elements.begin(),
                                                              cclt._refined_elements.end());
  _coarsened_elements = std::make_unique<ConstElemPointerRange>(cclt._coarsened_elements.begin(),
                                                                cclt._coarsened_elements.end());
  _coarsened_element_children = cclt._coarsened_element_children;
}

ConstElemPointerRange *
MooseMesh::refinedElementRange() const
{
  return _refined_elements.get();
}

ConstElemPointerRange *
MooseMesh::coarsenedElementRange() const
{
  return _coarsened_elements.get();
}

const std::vector<const Elem *> &
MooseMesh::coarsenedElementChildren(const Elem * elem) const
{
  auto elem_to_child_pair = _coarsened_element_children.find(elem);
  mooseAssert(elem_to_child_pair != _coarsened_element_children.end(), "Missing element in map");
  return elem_to_child_pair->second;
}

void
MooseMesh::updateActiveSemiLocalNodeRange(std::set<dof_id_type> & ghosted_elems)
{
  TIME_SECTION("updateActiveSemiLocalNodeRange", 5, "Updating ActiveSemiLocalNode Range");

  _semilocal_node_list.clear();

  // First add the nodes connected to local elems
  ConstElemRange * active_local_elems = getActiveLocalElementRange();
  for (const auto & elem : *active_local_elems)
  {
    for (unsigned int n = 0; n < elem->n_nodes(); ++n)
    {
      // Since elem is const here but we require a non-const Node * to
      // store in the _semilocal_node_list (otherwise things like
      // UpdateDisplacedMeshThread don't work), we are using a
      // const_cast. A more long-term fix would be to have
      // getActiveLocalElementRange return a non-const ElemRange.
      Node * node = const_cast<Node *>(elem->node_ptr(n));

      _semilocal_node_list.insert(node);
    }
  }

  // Now add the nodes connected to ghosted_elems
  for (const auto & ghost_elem_id : ghosted_elems)
  {
    Elem * elem = getMesh().elem_ptr(ghost_elem_id);
    for (unsigned int n = 0; n < elem->n_nodes(); n++)
    {
      Node * node = elem->node_ptr(n);

      _semilocal_node_list.insert(node);
    }
  }

  // Now create the actual range
  _active_semilocal_node_range = std::make_unique<SemiLocalNodeRange>(_semilocal_node_list.begin(),
                                                                      _semilocal_node_list.end());
}

bool
MooseMesh::isSemiLocal(Node * const node) const
{
  return _semilocal_node_list.find(node) != _semilocal_node_list.end();
}

/**
 * Helper class for sorting Boundary Nodes so that we always get the same
 * order of application for boundary conditions.
 */
class BndNodeCompare
{
public:
  BndNodeCompare() {}

  bool operator()(const BndNode * const & lhs, const BndNode * const & rhs)
  {
    if (lhs->_bnd_id < rhs->_bnd_id)
      return true;

    if (lhs->_bnd_id > rhs->_bnd_id)
      return false;

    if (lhs->_node->id() < rhs->_node->id())
      return true;

    if (lhs->_node->id() > rhs->_node->id())
      return false;

    return false;
  }
};

void
MooseMesh::buildNodeList()
{
  TIME_SECTION("buildNodeList", 5, "Building Node List");

  freeBndNodes();

  auto bc_tuples = getMesh().get_boundary_info().build_node_list();

  int n = bc_tuples.size();
  _bnd_nodes.clear();
  _bnd_nodes.reserve(n);
  for (const auto & t : bc_tuples)
  {
    auto node_id = std::get<0>(t);
    auto bc_id = std::get<1>(t);

    _bnd_nodes.push_back(new BndNode(getMesh().node_ptr(node_id), bc_id));
    _node_set_nodes[bc_id].push_back(node_id);
    _bnd_node_ids[bc_id].insert(node_id);
  }

  _bnd_nodes.reserve(_bnd_nodes.size() + _extra_bnd_nodes.size());
  for (unsigned int i = 0; i < _extra_bnd_nodes.size(); i++)
  {
    BndNode * bnode = new BndNode(_extra_bnd_nodes[i]._node, _extra_bnd_nodes[i]._bnd_id);
    _bnd_nodes.push_back(bnode);
    _bnd_node_ids[std::get<1>(bc_tuples[i])].insert(_extra_bnd_nodes[i]._node->id());
  }

  // This sort is here so that boundary conditions are always applied in the same order
  std::sort(_bnd_nodes.begin(), _bnd_nodes.end(), BndNodeCompare());
}

void
MooseMesh::buildElemIDInfo()
{
  unsigned int n = getMesh().n_elem_integers() + 1;

  _block_id_mapping.clear();
  _max_ids.clear();
  _min_ids.clear();
  _id_identical_flag.clear();

  _block_id_mapping.resize(n);
  _max_ids.resize(n, std::numeric_limits<dof_id_type>::min());
  _min_ids.resize(n, std::numeric_limits<dof_id_type>::max());
  _id_identical_flag.resize(n, std::vector<bool>(n, true));
  for (const auto & elem : getMesh().active_local_element_ptr_range())
    for (unsigned int i = 0; i < n; ++i)
    {
      auto id = (i == n - 1 ? elem->subdomain_id() : elem->get_extra_integer(i));
      _block_id_mapping[i][elem->subdomain_id()].insert(id);
      if (id > _max_ids[i])
        _max_ids[i] = id;
      if (id < _min_ids[i])
        _min_ids[i] = id;
      for (unsigned int j = 0; j < n; ++j)
      {
        auto idj = (j == n - 1 ? elem->subdomain_id() : elem->get_extra_integer(j));
        if (i != j && _id_identical_flag[i][j] && id != idj)
          _id_identical_flag[i][j] = false;
      }
    }

  for (unsigned int i = 0; i < n; ++i)
  {
    for (auto & blk : meshSubdomains())
      comm().set_union(_block_id_mapping[i][blk]);
    comm().min(_id_identical_flag[i]);
  }
  comm().max(_max_ids);
  comm().min(_min_ids);
}

std::set<dof_id_type>
MooseMesh::getAllElemIDs(unsigned int elem_id_index) const
{
  std::set<dof_id_type> unique_ids;
  for (auto & pair : _block_id_mapping[elem_id_index])
    for (auto & id : pair.second)
      unique_ids.insert(id);
  return unique_ids;
}

std::set<dof_id_type>
MooseMesh::getElemIDsOnBlocks(unsigned int elem_id_index, const std::set<SubdomainID> & blks) const
{
  std::set<dof_id_type> unique_ids;
  for (auto & blk : blks)
  {
    if (blk == Moose::ANY_BLOCK_ID)
      return getAllElemIDs(elem_id_index);

    auto it = _block_id_mapping[elem_id_index].find(blk);
    if (it == _block_id_mapping[elem_id_index].end())
      mooseError("Block ", blk, " is not available on the mesh");

    for (auto & mid : it->second)
      unique_ids.insert(mid);
  }
  return unique_ids;
}

void
MooseMesh::buildBndElemList()
{
  TIME_SECTION("buildBndElemList", 5, "Building Boundary Elements List");

  freeBndElems();

  auto bc_tuples = getMesh().get_boundary_info().build_active_side_list();

  int n = bc_tuples.size();
  _bnd_elems.clear();
  _bnd_elems.reserve(n);
  for (const auto & t : bc_tuples)
  {
    auto elem_id = std::get<0>(t);
    auto side_id = std::get<1>(t);
    auto bc_id = std::get<2>(t);

    _bnd_elems.push_back(new BndElement(getMesh().elem_ptr(elem_id), side_id, bc_id));
    _bnd_elem_ids[bc_id].insert(elem_id);
  }
}

const std::map<dof_id_type, std::vector<dof_id_type>> &
MooseMesh::nodeToElemMap()
{
  if (!_node_to_elem_map_built) // Guard the creation with a double checked lock
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

    if (!_node_to_elem_map_built)
    {
      // This is allowing the timing to be run even with threads
      // This is safe because all threads will be waiting on this section when it runs
      // NOTE: Do not copy this construction to other places without thinking REALLY hard about it
      // The PerfGraph is NOT threadsafe and will cause all kinds of havok if care isn't taken
      auto in_threads = Threads::in_threads;
      Threads::in_threads = false;
      TIME_SECTION("nodeToElemMap", 5, "Building Node To Elem Map");
      Threads::in_threads = in_threads;

      for (const auto & elem : getMesh().active_element_ptr_range())
        for (unsigned int n = 0; n < elem->n_nodes(); n++)
          _node_to_elem_map[elem->node_id(n)].push_back(elem->id());

      _node_to_elem_map_built = true; // MUST be set at the end for double-checked locking to work!
    }
  }

  return _node_to_elem_map;
}

const std::map<dof_id_type, std::vector<dof_id_type>> &
MooseMesh::nodeToActiveSemilocalElemMap()
{
  if (!_node_to_active_semilocal_elem_map_built) // Guard the creation with a double checked lock
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

    // This is allowing the timing to be run even with threads
    // This is safe because all threads will be waiting on this section when it runs
    // NOTE: Do not copy this construction to other places without thinking REALLY hard about it
    // The PerfGraph is NOT threadsafe and will cause all kinds of havok if care isn't taken
    auto in_threads = Threads::in_threads;
    Threads::in_threads = false;
    TIME_SECTION("nodeToActiveSemilocalElemMap", 5, "Building SemiLocalElemMap");
    Threads::in_threads = in_threads;

    if (!_node_to_active_semilocal_elem_map_built)
    {
      for (const auto & elem :
           as_range(getMesh().semilocal_elements_begin(), getMesh().semilocal_elements_end()))
        if (elem->active())
          for (unsigned int n = 0; n < elem->n_nodes(); n++)
            _node_to_active_semilocal_elem_map[elem->node_id(n)].push_back(elem->id());

      _node_to_active_semilocal_elem_map_built =
          true; // MUST be set at the end for double-checked locking to work!
    }
  }

  return _node_to_active_semilocal_elem_map;
}

ConstElemRange *
MooseMesh::getActiveLocalElementRange()
{
  if (!_active_local_elem_range)
  {
    TIME_SECTION("getActiveLocalElementRange", 5);

    _active_local_elem_range = std::make_unique<ConstElemRange>(
        getMesh().active_local_elements_begin(), getMesh().active_local_elements_end(), GRAIN_SIZE);
  }

  return _active_local_elem_range.get();
}

NodeRange *
MooseMesh::getActiveNodeRange()
{
  if (!_active_node_range)
  {
    TIME_SECTION("getActiveNodeRange", 5);

    _active_node_range = std::make_unique<NodeRange>(
        getMesh().active_nodes_begin(), getMesh().active_nodes_end(), GRAIN_SIZE);
  }

  return _active_node_range.get();
}

SemiLocalNodeRange *
MooseMesh::getActiveSemiLocalNodeRange() const
{
  mooseAssert(_active_semilocal_node_range,
              "_active_semilocal_node_range has not been created yet!");

  return _active_semilocal_node_range.get();
}

ConstNodeRange *
MooseMesh::getLocalNodeRange()
{
  if (!_local_node_range)
  {
    TIME_SECTION("getLocalNodeRange", 5);

    _local_node_range = std::make_unique<ConstNodeRange>(
        getMesh().local_nodes_begin(), getMesh().local_nodes_end(), GRAIN_SIZE);
  }

  return _local_node_range.get();
}

ConstBndNodeRange *
MooseMesh::getBoundaryNodeRange()
{
  if (!_bnd_node_range)
  {
    TIME_SECTION("getBoundaryNodeRange", 5);

    _bnd_node_range =
        std::make_unique<ConstBndNodeRange>(bndNodesBegin(), bndNodesEnd(), GRAIN_SIZE);
  }

  return _bnd_node_range.get();
}

ConstBndElemRange *
MooseMesh::getBoundaryElementRange()
{
  if (!_bnd_elem_range)
  {
    TIME_SECTION("getBoundaryElementRange", 5);

    _bnd_elem_range =
        std::make_unique<ConstBndElemRange>(bndElemsBegin(), bndElemsEnd(), GRAIN_SIZE);
  }

  return _bnd_elem_range.get();
}

const std::unordered_map<boundary_id_type, std::unordered_set<dof_id_type>> &
MooseMesh::getBoundariesToElems() const
{
  mooseDeprecated("MooseMesh::getBoundariesToElems is deprecated, "
                  "use MooseMesh::getBoundariesToActiveSemiLocalElemIds");
  return getBoundariesToActiveSemiLocalElemIds();
}

const std::unordered_map<boundary_id_type, std::unordered_set<dof_id_type>> &
MooseMesh::getBoundariesToActiveSemiLocalElemIds() const
{
  return _bnd_elem_ids;
}

std::unordered_set<dof_id_type>
MooseMesh::getBoundaryActiveSemiLocalElemIds(BoundaryID bid) const
{
  // The boundary to element map is computed on every mesh update
  const auto it = _bnd_elem_ids.find(bid);
  if (it == _bnd_elem_ids.end())
    // Boundary is not local to this domain, return an empty set
    return std::unordered_set<dof_id_type>{};
  return it->second;
}

std::unordered_set<dof_id_type>
MooseMesh::getBoundaryActiveNeighborElemIds(BoundaryID bid) const
{
  // Vector of boundary elems is updated every mesh update
  std::unordered_set<dof_id_type> neighbor_elems;
  for (const auto & bnd_elem : _bnd_elems)
  {
    const auto & [elem_ptr, elem_side, elem_bid] = *bnd_elem;
    if (elem_bid == bid)
    {
      const auto * neighbor = elem_ptr->neighbor_ptr(elem_side);
      // Dont add fully remote elements, ghosted is fine
      if (neighbor && neighbor != libMesh::remote_elem)
      {
        // handle mesh refinement, only return active elements near the boundary
        if (neighbor->active())
          neighbor_elems.insert(neighbor->id());
        else
        {
          std::vector<const Elem *> family;
          neighbor->active_family_tree_by_neighbor(family, elem_ptr);
          for (const auto & child_neighbor : family)
            neighbor_elems.insert(child_neighbor->id());
        }
      }
    }
  }

  return neighbor_elems;
}

bool
MooseMesh::isBoundaryFullyExternalToSubdomains(BoundaryID bid,
                                               const std::set<SubdomainID> & blk_group) const
{
  mooseAssert(_bnd_elem_range, "Boundary element range is not initialized");
  const bool all_blocks = blk_group.find(Moose::ANY_BLOCK_ID) != blk_group.end();

  // Loop over all side elements of the mesh, select those on the boundary
  for (const auto & bnd_elem : *_bnd_elem_range)
  {
    const auto & [elem_ptr, elem_side, elem_bid] = *bnd_elem;
    if (elem_bid == bid)
    {
      // If an element is internal to the group of subdomain, check the neighbor
      if (all_blocks || blk_group.find(elem_ptr->subdomain_id()) != blk_group.end())
      {
        const auto * const neighbor = elem_ptr->neighbor_ptr(elem_side);

        // If we did not ghost the neighbor, we cannot decide
        if (neighbor == libMesh::remote_elem)
          mooseError("Insufficient level of geometrical ghosting to determine "
                     "if a boundary is internal to the mesh");
        // If the neighbor does not exist, then we are on the edge of the mesh
        if (!neighbor)
          continue;
        // If the neighbor is also in the group of subdomain,
        // then the boundary cuts the subdomains
        if (all_blocks || blk_group.find(neighbor->subdomain_id()) != blk_group.end())
          return false;
      }
    }
  }
  return true;
}

void
MooseMesh::cacheInfo()
{
  TIME_SECTION("cacheInfo", 3);

  _sub_to_neighbor_subs.clear();
  _subdomain_boundary_ids.clear();
  _neighbor_subdomain_boundary_ids.clear();
  _block_node_list.clear();
  _higher_d_elem_side_to_lower_d_elem.clear();

  // TODO: Thread this!
  for (const auto & elem : getMesh().element_ptr_range())
  {
    const Elem * ip_elem = elem->interior_parent();

    if (ip_elem)
    {
      unsigned int ip_side = ip_elem->which_side_am_i(elem);

      // For some grid sequencing tests: ip_side == libMesh::invalid_uint
      if (ip_side != libMesh::invalid_uint)
      {
        auto pair = std::make_pair(ip_elem, ip_side);
        _higher_d_elem_side_to_lower_d_elem.insert(
            std::pair<std::pair<const Elem *, unsigned short int>, const Elem *>(pair, elem));
      }
    }

    for (unsigned int nd = 0; nd < elem->n_nodes(); ++nd)
    {
      Node & node = *elem->node_ptr(nd);
      _block_node_list[node.id()].insert(elem->subdomain_id());
    }
  }

  for (const auto & elem : getMesh().active_local_element_ptr_range())
  {
    SubdomainID subdomain_id = elem->subdomain_id();
    for (unsigned int side = 0; side < elem->n_sides(); side++)
    {
      std::vector<BoundaryID> boundary_ids = getBoundaryIDs(elem, side);
      std::set<BoundaryID> & subdomain_set = _subdomain_boundary_ids[subdomain_id];

      subdomain_set.insert(boundary_ids.begin(), boundary_ids.end());

      Elem * neig = elem->neighbor_ptr(side);
      if (neig)
      {
        _neighbor_subdomain_boundary_ids[neig->subdomain_id()].insert(boundary_ids.begin(),
                                                                      boundary_ids.end());
        SubdomainID neighbor_subdomain_id = neig->subdomain_id();
        if (neighbor_subdomain_id != subdomain_id)
          _sub_to_neighbor_subs[subdomain_id].insert(neighbor_subdomain_id);
      }
    }
  }

  for (const auto & blk_id : _mesh_subdomains)
  {
    _communicator.set_union(_sub_to_neighbor_subs[blk_id]);
    _communicator.set_union(_subdomain_boundary_ids[blk_id]);
    _communicator.set_union(_neighbor_subdomain_boundary_ids[blk_id]);
  }
}

const std::set<SubdomainID> &
MooseMesh::getNodeBlockIds(const Node & node) const
{
  std::map<dof_id_type, std::set<SubdomainID>>::const_iterator it =
      _block_node_list.find(node.id());

  if (it == _block_node_list.end())
    mooseError("Unable to find node: ", node.id(), " in any block list.");

  return it->second;
}

MooseMesh::face_info_iterator
MooseMesh::ownedFaceInfoBegin()
{
  return face_info_iterator(
      _face_info.begin(),
      _face_info.end(),
      libMesh::Predicates::pid<std::vector<const FaceInfo *>::iterator>(this->processor_id()));
}

MooseMesh::face_info_iterator
MooseMesh::ownedFaceInfoEnd()
{
  return face_info_iterator(
      _face_info.end(),
      _face_info.end(),
      libMesh::Predicates::pid<std::vector<const FaceInfo *>::iterator>(this->processor_id()));
}

// default begin() accessor
MooseMesh::bnd_node_iterator
MooseMesh::bndNodesBegin()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.begin(), _bnd_nodes.end(), p);
}

// default end() accessor
MooseMesh::bnd_node_iterator
MooseMesh::bndNodesEnd()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.end(), _bnd_nodes.end(), p);
}

// default begin() accessor
MooseMesh::bnd_elem_iterator
MooseMesh::bndElemsBegin()
{
  Predicates::NotNull<bnd_elem_iterator_imp> p;
  return bnd_elem_iterator(_bnd_elems.begin(), _bnd_elems.end(), p);
}

// default end() accessor
MooseMesh::bnd_elem_iterator
MooseMesh::bndElemsEnd()
{
  Predicates::NotNull<bnd_elem_iterator_imp> p;
  return bnd_elem_iterator(_bnd_elems.end(), _bnd_elems.end(), p);
}

const Node *
MooseMesh::addUniqueNode(const Point & p, Real tol)
{
  /**
   * Looping through the mesh nodes each time we add a point is very slow.  To speed things
   * up we keep a local data structure
   */
  if (getMesh().n_nodes() != _node_map.size())
  {
    _node_map.clear();
    _node_map.reserve(getMesh().n_nodes());
    for (const auto & node : getMesh().node_ptr_range())
      _node_map.push_back(node);
  }

  Node * node = nullptr;
  for (unsigned int i = 0; i < _node_map.size(); ++i)
  {
    if (p.relative_fuzzy_equals(*_node_map[i], tol))
    {
      node = _node_map[i];
      break;
    }
  }
  if (node == nullptr)
  {
    node = getMesh().add_node(new Node(p));
    _node_map.push_back(node);
  }

  mooseAssert(node != nullptr, "Node is NULL");
  return node;
}

Node *
MooseMesh::addQuadratureNode(const Elem * elem,
                             const unsigned short int side,
                             const unsigned int qp,
                             BoundaryID bid,
                             const Point & point)
{
  Node * qnode;

  if (_elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].find(qp) ==
      _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].end())
  {
    // Create a new node id starting from the max node id and counting down.  This will be the least
    // likely to collide with an existing node id.
    // Note that we are using numeric_limits<unsigned>::max even
    // though max_id is stored as a dof_id_type.  I tried this with
    // numeric_limits<dof_id_type>::max and it broke several tests in
    // MOOSE.  So, this is some kind of a magic number that we will
    // just continue to use...
    dof_id_type max_id = std::numeric_limits<unsigned int>::max() - 100;
    dof_id_type new_id = max_id - _quadrature_nodes.size();

    if (new_id <= getMesh().max_node_id())
      mooseError("Quadrature node id collides with existing node id!");

    qnode = new Node(point, new_id);

    // Keep track of this new node in two different ways for easy lookup
    _quadrature_nodes[new_id] = qnode;
    _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side][qp] = qnode;

    if (elem->active())
    {
      _node_to_elem_map[new_id].push_back(elem->id());
      _node_to_active_semilocal_elem_map[new_id].push_back(elem->id());
    }
  }
  else
    qnode = _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side][qp];

  BndNode * bnode = new BndNode(qnode, bid);
  _bnd_nodes.push_back(bnode);
  _bnd_node_ids[bid].insert(qnode->id());

  _extra_bnd_nodes.push_back(*bnode);

  // Do this so the range will be regenerated next time it is accessed
  _bnd_node_range.reset();

  return qnode;
}

Node *
MooseMesh::getQuadratureNode(const Elem * elem,
                             const unsigned short int side,
                             const unsigned int qp)
{
  mooseAssert(_elem_to_side_to_qp_to_quadrature_nodes.find(elem->id()) !=
                  _elem_to_side_to_qp_to_quadrature_nodes.end(),
              "Elem has no quadrature nodes!");
  mooseAssert(_elem_to_side_to_qp_to_quadrature_nodes[elem->id()].find(side) !=
                  _elem_to_side_to_qp_to_quadrature_nodes[elem->id()].end(),
              "Side has no quadrature nodes!");
  mooseAssert(_elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].find(qp) !=
                  _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].end(),
              "qp not found on side!");

  return _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side][qp];
}

void
MooseMesh::clearQuadratureNodes()
{
  // Delete all the quadrature nodes
  for (auto & it : _quadrature_nodes)
    delete it.second;

  _quadrature_nodes.clear();
  _elem_to_side_to_qp_to_quadrature_nodes.clear();
  _extra_bnd_nodes.clear();
}

BoundaryID
MooseMesh::getBoundaryID(const BoundaryName & boundary_name) const
{
  if (boundary_name == "ANY_BOUNDARY_ID")
    mooseError("Please use getBoundaryIDs() when passing \"ANY_BOUNDARY_ID\"");

  return MooseMeshUtils::getBoundaryID(boundary_name, getMesh());
}

const Elem *
MooseMesh::getLowerDElem(const Elem * elem, unsigned short int side) const
{
  auto it = _higher_d_elem_side_to_lower_d_elem.find(std::make_pair(elem, side));

  if (it != _higher_d_elem_side_to_lower_d_elem.end())
    return it->second;
  else
    return nullptr;
}

unsigned int
MooseMesh::getHigherDSide(const Elem * elem) const
{
  auto it = _lower_d_elem_to_higher_d_elem_side.find(elem);

  if (it != _lower_d_elem_to_higher_d_elem_side.end())
    return it->second;
  else
    return libMesh::invalid_uint;
}

std::vector<BoundaryID>
MooseMesh::getBoundaryIDs(const std::vector<BoundaryName> & boundary_name,
                          bool generate_unknown) const
{
  return MooseMeshUtils::getBoundaryIDs(
      getMesh(), boundary_name, generate_unknown, _mesh_boundary_ids);
}

SubdomainID
MooseMesh::getSubdomainID(const SubdomainName & subdomain_name) const
{
  if (subdomain_name == "ANY_BLOCK_ID")
    mooseError("Please use getSubdomainIDs() when passing \"ANY_BLOCK_ID\"");

  return MooseMeshUtils::getSubdomainID(subdomain_name, getMesh());
}

std::vector<SubdomainID>
MooseMesh::getSubdomainIDs(const std::vector<SubdomainName> & subdomain_name) const
{
  return MooseMeshUtils::getSubdomainIDs(getMesh(), subdomain_name, _mesh_subdomains);
}

void
MooseMesh::setSubdomainName(SubdomainID subdomain_id, const SubdomainName & name)
{
  getMesh().subdomain_name(subdomain_id) = name;
}

void
MooseMesh::setSubdomainName(MeshBase & mesh, SubdomainID subdomain_id, const SubdomainName & name)
{
  mesh.subdomain_name(subdomain_id) = name;
}

const std::string &
MooseMesh::getSubdomainName(SubdomainID subdomain_id) const
{
  return getMesh().subdomain_name(subdomain_id);
}

std::vector<SubdomainName>
MooseMesh::getSubdomainNames(const std::vector<SubdomainID> & subdomain_ids) const
{
  std::vector<SubdomainName> names(subdomain_ids.size());

  for (unsigned int i = 0; i < subdomain_ids.size(); i++)
  {
    if (subdomain_ids[i] == Moose::ANY_BLOCK_ID)
    {
      unsigned int j = 0;
      for (const auto & sub_id : _mesh_subdomains)
        names[j++] = getSubdomainName(sub_id);
      if (i)
        mooseWarning("You passed \"ANY_BLOCK_ID\" in addition to other block ids. This may be a "
                     "logic error.");
      break;
    }

    names[i] = getSubdomainName(subdomain_ids[i]);
  }

  return names;
}

void
MooseMesh::setBoundaryName(BoundaryID boundary_id, BoundaryName name)
{
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  // We need to figure out if this boundary is a sideset or nodeset
  if (boundary_info.get_side_boundary_ids().count(boundary_id))
    boundary_info.sideset_name(boundary_id) = name;
  else
    boundary_info.nodeset_name(boundary_id) = name;
}

const std::string &
MooseMesh::getBoundaryName(BoundaryID boundary_id)
{
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  // We need to figure out if this boundary is a sideset or nodeset
  if (boundary_info.get_side_boundary_ids().count(boundary_id))
    return boundary_info.get_sideset_name(boundary_id);
  else
    return boundary_info.get_nodeset_name(boundary_id);
}

// specialization for PointListAdaptor<MooseMesh::PeriodicNodeInfo>
template <>
inline const Point &
PointListAdaptor<MooseMesh::PeriodicNodeInfo>::getPoint(
    const MooseMesh::PeriodicNodeInfo & item) const
{
  return *(item.first);
}

void
MooseMesh::buildPeriodicNodeMap(std::multimap<dof_id_type, dof_id_type> & periodic_node_map,
                                unsigned int var_number,
                                PeriodicBoundaries * pbs) const
{
  TIME_SECTION("buildPeriodicNodeMap", 5);

  // clear existing map
  periodic_node_map.clear();

  // get periodic nodes
  std::vector<PeriodicNodeInfo> periodic_nodes;
  for (const auto & t : getMesh().get_boundary_info().build_node_list())
  {
    // unfortunately libMesh does not give us a pointer, so we have to look it up ourselves
    auto node = _mesh->node_ptr(std::get<0>(t));
    mooseAssert(node != nullptr,
                "libMesh::BoundaryInfo::build_node_list() returned an ID for a non-existing node");
    auto bc_id = std::get<1>(t);
    periodic_nodes.emplace_back(node, bc_id);
  }

  // sort by boundary id
  std::sort(periodic_nodes.begin(),
            periodic_nodes.end(),
            [](const PeriodicNodeInfo & a, const PeriodicNodeInfo & b) -> bool
            { return a.second > b.second; });

  // build kd-tree
  using KDTreeType = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<Real, PointListAdaptor<PeriodicNodeInfo>, Real, std::size_t>,
      PointListAdaptor<PeriodicNodeInfo>,
      LIBMESH_DIM,
      std::size_t>;
  const unsigned int max_leaf_size = 20; // slightly affects runtime
  auto point_list =
      PointListAdaptor<PeriodicNodeInfo>(periodic_nodes.begin(), periodic_nodes.end());
  auto kd_tree = std::make_unique<KDTreeType>(
      LIBMESH_DIM, point_list, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size));
  mooseAssert(kd_tree != nullptr, "KDTree was not properly initialized.");
  kd_tree->buildIndex();

  // data structures for kd-tree search
  nanoflann::SearchParams search_params;
  std::vector<std::pair<std::size_t, Real>> ret_matches;

  // iterate over periodic nodes (boundary ids are in contiguous blocks)
  PeriodicBoundaryBase * periodic = nullptr;
  BoundaryID current_bc_id = BoundaryInfo::invalid_id;
  for (auto & pair : periodic_nodes)
  {
    // entering a new block of boundary IDs
    if (pair.second != current_bc_id)
    {
      current_bc_id = pair.second;
      periodic = pbs->boundary(current_bc_id);
      if (periodic && !periodic->is_my_variable(var_number))
        periodic = nullptr;
    }

    // variable is not periodic at this node, skip
    if (!periodic)
      continue;

    // clear result buffer
    ret_matches.clear();

    // id of the current node
    const auto id = pair.first->id();

    // position where we expect a periodic partner for the current node and boundary
    Point search_point = periodic->get_corresponding_pos(*pair.first);

    // search at the expected point
    kd_tree->radiusSearch(&(search_point)(0), libMesh::TOLERANCE, ret_matches, search_params);
    for (auto & match_pair : ret_matches)
    {
      const auto & match = periodic_nodes[match_pair.first];
      // add matched node if the boundary id is the corresponding id in the periodic pair
      if (match.second == periodic->pairedboundary)
        periodic_node_map.emplace(id, match.first->id());
    }
  }
}

void
MooseMesh::buildPeriodicNodeSets(std::map<BoundaryID, std::set<dof_id_type>> & periodic_node_sets,
                                 unsigned int var_number,
                                 PeriodicBoundaries * pbs) const
{
  TIME_SECTION("buildPeriodicNodeSets", 5);

  periodic_node_sets.clear();

  // Loop over all the boundary nodes adding the periodic nodes to the appropriate set
  for (const auto & t : getMesh().get_boundary_info().build_node_list())
  {
    auto node_id = std::get<0>(t);
    auto bc_id = std::get<1>(t);

    // Is this current node on a known periodic boundary?
    if (periodic_node_sets.find(bc_id) != periodic_node_sets.end())
      periodic_node_sets[bc_id].insert(node_id);
    else // This still might be a periodic node but we just haven't seen this boundary_id yet
    {
      const PeriodicBoundaryBase * periodic = pbs->boundary(bc_id);
      if (periodic && periodic->is_my_variable(var_number))
        periodic_node_sets[bc_id].insert(node_id);
    }
  }
}

bool
MooseMesh::detectOrthogonalDimRanges(Real tol)
{
  TIME_SECTION("detectOrthogonalDimRanges", 5);

  if (_regular_orthogonal_mesh)
    return true;

  std::vector<Real> min(3, std::numeric_limits<Real>::max());
  std::vector<Real> max(3, std::numeric_limits<Real>::min());
  unsigned int dim = getMesh().mesh_dimension();

  // Find the bounding box of our mesh
  for (const auto & node : getMesh().node_ptr_range())
    // Check all coordinates, we don't know if this mesh might be lying in a higher dim even if the
    // mesh dimension is lower.
    for (const auto i : make_range(Moose::dim))
    {
      if ((*node)(i) < min[i])
        min[i] = (*node)(i);
      if ((*node)(i) > max[i])
        max[i] = (*node)(i);
    }

  this->comm().max(max);
  this->comm().min(min);

  _extreme_nodes.resize(8); // 2^LIBMESH_DIM
  // Now make sure that there are actual nodes at all of the extremes
  std::vector<bool> extreme_matches(8, false);
  std::vector<unsigned int> comp_map(3);
  for (const auto & node : getMesh().node_ptr_range())
  {
    // See if the current node is located at one of the extremes
    unsigned int coord_match = 0;

    for (const auto i : make_range(Moose::dim))
    {
      if (std::abs((*node)(i)-min[i]) < tol)
      {
        comp_map[i] = MIN;
        ++coord_match;
      }
      else if (std::abs((*node)(i)-max[i]) < tol)
      {
        comp_map[i] = MAX;
        ++coord_match;
      }
    }

    if (coord_match == LIBMESH_DIM) // Found a coordinate at one of the extremes
    {
      _extreme_nodes[comp_map[X] * 4 + comp_map[Y] * 2 + comp_map[Z]] = node;
      extreme_matches[comp_map[X] * 4 + comp_map[Y] * 2 + comp_map[Z]] = true;
    }
  }

  // See if we matched all of the extremes for the mesh dimension
  this->comm().max(extreme_matches);
  if (std::count(extreme_matches.begin(), extreme_matches.end(), true) == (1 << dim))
    _regular_orthogonal_mesh = true;

  // Set the bounds
  _bounds.resize(LIBMESH_DIM);
  for (const auto i : make_range(Moose::dim))
  {
    _bounds[i].resize(2);
    _bounds[i][MIN] = min[i];
    _bounds[i][MAX] = max[i];
  }

  return _regular_orthogonal_mesh;
}

void
MooseMesh::detectPairedSidesets()
{
  TIME_SECTION("detectPairedSidesets", 5);

  // Loop over level-0 elements (since boundary condition information
  // is only directly stored for them) and find sidesets with normals
  // that point in the -x, +x, -y, +y, and -z, +z direction.  If there
  // is a unique sideset id for each direction, then the paired
  // sidesets consist of (-x,+x), (-y,+y), (-z,+z).  If there are
  // multiple sideset ids for a given direction, then we can't pick a
  // single pair for that direction.  In that case, we'll just return
  // as was done in the original algorithm.

  // Points used for direction comparison
  const Point minus_x(-1, 0, 0), plus_x(1, 0, 0), minus_y(0, -1, 0), plus_y(0, 1, 0),
      minus_z(0, 0, -1), plus_z(0, 0, 1);

  // we need to test all element dimensions from dim down to 1
  const unsigned int dim = getMesh().mesh_dimension();

  // boundary id sets for elements of different dimensions
  std::vector<std::set<BoundaryID>> minus_x_ids(dim), plus_x_ids(dim), minus_y_ids(dim),
      plus_y_ids(dim), minus_z_ids(dim), plus_z_ids(dim);

  std::vector<std::unique_ptr<FEBase>> fe_faces(dim);
  std::vector<std::unique_ptr<QGauss>> qfaces(dim);
  for (unsigned side_dim = 0; side_dim < dim; ++side_dim)
  {
    // Face is assumed to be flat, therefore normal is assumed to be
    // constant over the face, therefore only compute it at 1 qp.
    qfaces[side_dim] = std::unique_ptr<QGauss>(new QGauss(side_dim, CONSTANT));

    // A first-order Lagrange FE for the face.
    fe_faces[side_dim] = FEBase::build(side_dim + 1, FEType(FIRST, LAGRANGE));
    fe_faces[side_dim]->attach_quadrature_rule(qfaces[side_dim].get());
  }

  // We need this to get boundary ids for each boundary face we encounter.
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();
  std::vector<boundary_id_type> face_ids;

  for (auto & elem : as_range(getMesh().level_elements_begin(0), getMesh().level_elements_end(0)))
  {
    // dimension of the current element and its normals
    unsigned int side_dim = elem->dim() - 1;
    const std::vector<Point> & normals = fe_faces[side_dim]->get_normals();

    // loop over element sides
    for (unsigned int s = 0; s < elem->n_sides(); s++)
    {
      // If side is on the boundary
      if (elem->neighbor_ptr(s) == nullptr)
      {
        std::unique_ptr<Elem> side = elem->build_side_ptr(s);

        fe_faces[side_dim]->reinit(elem, s);

        // Get the boundary ID(s) for this side.  If there is more
        // than 1 boundary id, then we already can't determine a
        // unique pairing of sides in this direction, but we'll just
        // keep going to keep the logic simple.
        boundary_info.boundary_ids(elem, s, face_ids);

        // x-direction faces
        if (normals[0].absolute_fuzzy_equals(minus_x))
          minus_x_ids[side_dim].insert(face_ids.begin(), face_ids.end());
        else if (normals[0].absolute_fuzzy_equals(plus_x))
          plus_x_ids[side_dim].insert(face_ids.begin(), face_ids.end());

        // y-direction faces
        else if (normals[0].absolute_fuzzy_equals(minus_y))
          minus_y_ids[side_dim].insert(face_ids.begin(), face_ids.end());
        else if (normals[0].absolute_fuzzy_equals(plus_y))
          plus_y_ids[side_dim].insert(face_ids.begin(), face_ids.end());

        // z-direction faces
        else if (normals[0].absolute_fuzzy_equals(minus_z))
          minus_z_ids[side_dim].insert(face_ids.begin(), face_ids.end());
        else if (normals[0].absolute_fuzzy_equals(plus_z))
          plus_z_ids[side_dim].insert(face_ids.begin(), face_ids.end());
      }
    }
  }

  // For a distributed mesh, boundaries may be distributed as well. We therefore collect information
  // from everyone. If the mesh is already serial, then there is no need to do an allgather. Note
  // that this is just going to gather information about what the periodic bc ids are. We are not
  // gathering any remote elements or anything like that. It's just that the GhostPointNeighbors
  // ghosting functor currently relies on the fact that every process agrees on whether we have
  // periodic boundaries; every process that thinks there are periodic boundaries will call
  // MeshBase::sub_point_locator which makes a parallel_object_only() assertion (right or wrong). So
  // we all need to go there (or not go there)
  if (_use_distributed_mesh && !_mesh->is_serial())
  {
    // Pack all data together so that we send them via one communication
    // pair: boundary side --> boundary ids.
    std::vector<std::pair<boundary_id_type, boundary_id_type>> vecdata;
    //  We check boundaries on all dimensions
    for (unsigned side_dim = 0; side_dim < dim; ++side_dim)
    {
      // "6" means: we have at most 6 boundaries. It is true for generated simple mesh
      // "detectPairedSidesets" is designed for only simple meshes
      for (auto bd = minus_x_ids[side_dim].begin(); bd != minus_x_ids[side_dim].end(); bd++)
        vecdata.emplace_back(side_dim * 6 + 0, *bd);

      for (auto bd = plus_x_ids[side_dim].begin(); bd != plus_x_ids[side_dim].end(); bd++)
        vecdata.emplace_back(side_dim * 6 + 1, *bd);

      for (auto bd = minus_y_ids[side_dim].begin(); bd != minus_y_ids[side_dim].end(); bd++)
        vecdata.emplace_back(side_dim * 6 + 2, *bd);

      for (auto bd = plus_y_ids[side_dim].begin(); bd != plus_y_ids[side_dim].end(); bd++)
        vecdata.emplace_back(side_dim * 6 + 3, *bd);

      for (auto bd = minus_z_ids[side_dim].begin(); bd != minus_z_ids[side_dim].end(); bd++)
        vecdata.emplace_back(side_dim * 6 + 4, *bd);

      for (auto bd = plus_z_ids[side_dim].begin(); bd != plus_z_ids[side_dim].end(); bd++)
        vecdata.emplace_back(side_dim * 6 + 5, *bd);
    }

    _communicator.allgather(vecdata, false);

    // Unpack data, and add them into minus/plus_x/y_ids
    for (auto pair = vecdata.begin(); pair != vecdata.end(); pair++)
    {
      // Convert data from the long vector, and add data to separated sets
      auto side_dim = pair->first / 6;
      auto side = pair->first % 6;

      switch (side)
      {
        case 0:
          minus_x_ids[side_dim].insert(pair->second);
          break;
        case 1:
          plus_x_ids[side_dim].insert(pair->second);
          break;
        case 2:
          minus_y_ids[side_dim].insert(pair->second);
          break;
        case 3:
          plus_y_ids[side_dim].insert(pair->second);
          break;
        case 4:
          minus_z_ids[side_dim].insert(pair->second);
          break;
        case 5:
          plus_z_ids[side_dim].insert(pair->second);
          break;
        default:
          mooseError("Unknown boundary side ", side);
      }
    }

  } // end if (_use_distributed_mesh && !_need_ghost_ghosted_boundaries)

  for (unsigned side_dim = 0; side_dim < dim; ++side_dim)
  {
    // If unique pairings were found, fill up the _paired_boundary data
    // structure with that information.
    if (minus_x_ids[side_dim].size() == 1 && plus_x_ids[side_dim].size() == 1)
      _paired_boundary.emplace_back(
          std::make_pair(*(minus_x_ids[side_dim].begin()), *(plus_x_ids[side_dim].begin())));

    if (minus_y_ids[side_dim].size() == 1 && plus_y_ids[side_dim].size() == 1)
      _paired_boundary.emplace_back(
          std::make_pair(*(minus_y_ids[side_dim].begin()), *(plus_y_ids[side_dim].begin())));

    if (minus_z_ids[side_dim].size() == 1 && plus_z_ids[side_dim].size() == 1)
      _paired_boundary.emplace_back(
          std::make_pair(*(minus_z_ids[side_dim].begin()), *(plus_z_ids[side_dim].begin())));
  }
}

Real
MooseMesh::dimensionWidth(unsigned int component) const
{
  return getMaxInDimension(component) - getMinInDimension(component);
}

Real
MooseMesh::getMinInDimension(unsigned int component) const
{
  mooseAssert(_mesh, "The MeshBase has not been constructed");
  mooseAssert(component < _bounds.size(), "Requested dimension out of bounds");

  return _bounds[component][MIN];
}

Real
MooseMesh::getMaxInDimension(unsigned int component) const
{
  mooseAssert(_mesh, "The MeshBase has not been constructed");
  mooseAssert(component < _bounds.size(), "Requested dimension out of bounds");

  return _bounds[component][MAX];
}

void
MooseMesh::addPeriodicVariable(unsigned int var_num, BoundaryID primary, BoundaryID secondary)
{
  if (!_regular_orthogonal_mesh)
    return;

  _periodic_dim[var_num].resize(dimension());

  _half_range = Point(dimensionWidth(0) / 2.0, dimensionWidth(1) / 2.0, dimensionWidth(2) / 2.0);

  for (unsigned int component = 0; component < dimension(); ++component)
  {
    const std::pair<BoundaryID, BoundaryID> * boundary_ids = getPairedBoundaryMapping(component);

    if (boundary_ids != nullptr &&
        ((boundary_ids->first == primary && boundary_ids->second == secondary) ||
         (boundary_ids->first == secondary && boundary_ids->second == primary)))
      _periodic_dim[var_num][component] = true;
  }
}

bool
MooseMesh::isTranslatedPeriodic(unsigned int nonlinear_var_num, unsigned int component) const
{
  mooseAssert(component < dimension(), "Requested dimension out of bounds");

  if (_periodic_dim.find(nonlinear_var_num) != _periodic_dim.end())
    return _periodic_dim.at(nonlinear_var_num)[component];
  else
    return false;
}

RealVectorValue
MooseMesh::minPeriodicVector(unsigned int nonlinear_var_num, Point p, Point q) const
{
  for (unsigned int i = 0; i < dimension(); ++i)
  {
    // check to see if we're closer in real or periodic space in x, y, and z
    if (isTranslatedPeriodic(nonlinear_var_num, i))
    {
      // Need to test order before differencing
      if (p(i) > q(i))
      {
        if (p(i) - q(i) > _half_range(i))
          p(i) -= _half_range(i) * 2;
      }
      else
      {
        if (q(i) - p(i) > _half_range(i))
          p(i) += _half_range(i) * 2;
      }
    }
  }

  return q - p;
}

Real
MooseMesh::minPeriodicDistance(unsigned int nonlinear_var_num, Point p, Point q) const
{
  return minPeriodicVector(nonlinear_var_num, p, q).norm();
}

const std::pair<BoundaryID, BoundaryID> *
MooseMesh::getPairedBoundaryMapping(unsigned int component)
{
  if (!_regular_orthogonal_mesh)
    mooseError("Trying to retrieve automatic paired mapping for a mesh that is not regular and "
               "orthogonal");

  mooseAssert(component < dimension(), "Requested dimension out of bounds");

  if (_paired_boundary.empty())
    detectPairedSidesets();

  if (component < _paired_boundary.size())
    return &_paired_boundary[component];
  else
    return nullptr;
}

void
MooseMesh::buildRefinementAndCoarseningMaps(Assembly * assembly)
{
  TIME_SECTION("buildRefinementAndCoarseningMaps", 5, "Building Refinement And Coarsening Maps");

  std::map<ElemType, Elem *> canonical_elems;

  // First, loop over all elements and find a canonical element for each type
  // Doing it this way guarantees that this is going to work in parallel
  for (const auto & elem : getMesh().element_ptr_range()) // TODO: Thread this
  {
    ElemType type = elem->type();

    if (canonical_elems.find(type) ==
        canonical_elems.end()) // If we haven't seen this type of elem before save it
      canonical_elems[type] = elem;
    else
    {
      Elem * stored = canonical_elems[type];
      if (elem->id() < stored->id()) // Arbitrarily keep the one with a lower id
        canonical_elems[type] = elem;
    }
  }
  // Now build the maps using these templates
  // Note: This MUST be done NOT threaded!
  for (const auto & can_it : canonical_elems)
  {
    Elem * elem = can_it.second;

    // Need to do this just once to get the right qrules put in place
    assembly->setCurrentSubdomainID(elem->subdomain_id());
    assembly->reinit(elem);
    assembly->reinit(elem, 0);
    auto && qrule = assembly->writeableQRule();
    auto && qrule_face = assembly->writeableQRuleFace();

    // Volume to volume projection for refinement
    buildRefinementMap(*elem, *qrule, *qrule_face, -1, -1, -1);

    // Volume to volume projection for coarsening
    buildCoarseningMap(*elem, *qrule, *qrule_face, -1);

    // Map the sides of children
    for (unsigned int side = 0; side < elem->n_sides(); side++)
    {
      // Side to side for sides that match parent's sides
      buildRefinementMap(*elem, *qrule, *qrule_face, side, -1, side);
      buildCoarseningMap(*elem, *qrule, *qrule_face, side);
    }

    // Child side to parent volume mapping for "internal" child sides
    for (unsigned int child = 0; child < elem->n_children(); ++child)
      for (unsigned int side = 0; side < elem->n_sides();
           ++side)                                // Assume children have the same number of sides!
        if (!elem->is_child_on_side(child, side)) // Otherwise we already computed that map
          buildRefinementMap(*elem, *qrule, *qrule_face, -1, child, side);
  }
}

void
MooseMesh::buildRefinementMap(const Elem & elem,
                              QBase & qrule,
                              QBase & qrule_face,
                              int parent_side,
                              int child,
                              int child_side)
{
  TIME_SECTION("buildRefinementMap", 5, "Building Refinement Map");

  if (child == -1) // Doing volume mapping or parent side mapping
  {
    mooseAssert(parent_side == child_side,
                "Parent side must match child_side if not passing a specific child!");

    std::pair<int, ElemType> the_pair(parent_side, elem.type());

    if (_elem_type_to_refinement_map.find(the_pair) != _elem_type_to_refinement_map.end())
      mooseError("Already built a qp refinement map!");

    std::vector<std::pair<unsigned int, QpMap>> coarsen_map;
    std::vector<std::vector<QpMap>> & refinement_map = _elem_type_to_refinement_map[the_pair];
    findAdaptivityQpMaps(
        &elem, qrule, qrule_face, refinement_map, coarsen_map, parent_side, child, child_side);
  }
  else // Need to map a child side to parent volume qps
  {
    std::pair<int, int> child_pair(child, child_side);

    if (_elem_type_to_child_side_refinement_map.find(elem.type()) !=
            _elem_type_to_child_side_refinement_map.end() &&
        _elem_type_to_child_side_refinement_map[elem.type()].find(child_pair) !=
            _elem_type_to_child_side_refinement_map[elem.type()].end())
      mooseError("Already built a qp refinement map!");

    std::vector<std::pair<unsigned int, QpMap>> coarsen_map;
    std::vector<std::vector<QpMap>> & refinement_map =
        _elem_type_to_child_side_refinement_map[elem.type()][child_pair];
    findAdaptivityQpMaps(
        &elem, qrule, qrule_face, refinement_map, coarsen_map, parent_side, child, child_side);
  }
}

const std::vector<std::vector<QpMap>> &
MooseMesh::getRefinementMap(const Elem & elem, int parent_side, int child, int child_side)
{
  if (child == -1) // Doing volume mapping or parent side mapping
  {
    mooseAssert(parent_side == child_side,
                "Parent side must match child_side if not passing a specific child!");

    std::pair<int, ElemType> the_pair(parent_side, elem.type());

    if (_elem_type_to_refinement_map.find(the_pair) == _elem_type_to_refinement_map.end())
      mooseError("Could not find a suitable qp refinement map!");

    return _elem_type_to_refinement_map[the_pair];
  }
  else // Need to map a child side to parent volume qps
  {
    std::pair<int, int> child_pair(child, child_side);

    if (_elem_type_to_child_side_refinement_map.find(elem.type()) ==
            _elem_type_to_child_side_refinement_map.end() ||
        _elem_type_to_child_side_refinement_map[elem.type()].find(child_pair) ==
            _elem_type_to_child_side_refinement_map[elem.type()].end())
      mooseError("Could not find a suitable qp refinement map!");

    return _elem_type_to_child_side_refinement_map[elem.type()][child_pair];
  }

  /**
   *  TODO: When running with parallel mesh + stateful adaptivty we will need to make sure that each
   *  processor has a complete map.  This may require parallel communication.  This is likely to
   * happen
   *  when running on a mixed element mesh.
   */
}

void
MooseMesh::buildCoarseningMap(const Elem & elem, QBase & qrule, QBase & qrule_face, int input_side)
{
  TIME_SECTION("buildCoarseningMap", 5, "Building Coarsening Map");

  std::pair<int, ElemType> the_pair(input_side, elem.type());

  if (_elem_type_to_coarsening_map.find(the_pair) != _elem_type_to_coarsening_map.end())
    mooseError("Already built a qp coarsening map!");

  std::vector<std::vector<QpMap>> refinement_map;
  std::vector<std::pair<unsigned int, QpMap>> & coarsen_map =
      _elem_type_to_coarsening_map[the_pair];

  // The -1 here is for a specific child.  We don't do that for coarsening maps
  // Also note that we're always mapping the same side to the same side (which is guaranteed by
  // libMesh).
  findAdaptivityQpMaps(
      &elem, qrule, qrule_face, refinement_map, coarsen_map, input_side, -1, input_side);

  /**
   *  TODO: When running with parallel mesh + stateful adaptivty we will need to make sure that each
   *  processor has a complete map.  This may require parallel communication.  This is likely to
   * happen
   *  when running on a mixed element mesh.
   */
}

const std::vector<std::pair<unsigned int, QpMap>> &
MooseMesh::getCoarseningMap(const Elem & elem, int input_side)
{
  std::pair<int, ElemType> the_pair(input_side, elem.type());

  if (_elem_type_to_coarsening_map.find(the_pair) == _elem_type_to_coarsening_map.end())
    mooseError("Could not find a suitable qp refinement map!");

  return _elem_type_to_coarsening_map[the_pair];
}

void
MooseMesh::mapPoints(const std::vector<Point> & from,
                     const std::vector<Point> & to,
                     std::vector<QpMap> & qp_map)
{
  unsigned int n_from = from.size();
  unsigned int n_to = to.size();

  qp_map.resize(n_from);

  for (unsigned int i = 0; i < n_from; ++i)
  {
    const Point & from_point = from[i];

    QpMap & current_map = qp_map[i];

    for (unsigned int j = 0; j < n_to; ++j)
    {
      const Point & to_point = to[j];
      Real distance = (from_point - to_point).norm();

      if (distance < current_map._distance)
      {
        current_map._distance = distance;
        current_map._from = i;
        current_map._to = j;
      }
    }
  }
}

void
MooseMesh::findAdaptivityQpMaps(const Elem * template_elem,
                                QBase & qrule,
                                QBase & qrule_face,
                                std::vector<std::vector<QpMap>> & refinement_map,
                                std::vector<std::pair<unsigned int, QpMap>> & coarsen_map,
                                int parent_side,
                                int child,
                                int child_side)
{
  TIME_SECTION("findAdaptivityQpMaps", 5);

  ReplicatedMesh mesh(_communicator);
  mesh.skip_partitioning(true);

  unsigned int dim = template_elem->dim();
  mesh.set_mesh_dimension(dim);

  for (unsigned int i = 0; i < template_elem->n_nodes(); ++i)
    mesh.add_point(template_elem->point(i));

  Elem * elem = mesh.add_elem(Elem::build(template_elem->type()).release());

  for (unsigned int i = 0; i < template_elem->n_nodes(); ++i)
    elem->set_node(i) = mesh.node_ptr(i);

  std::unique_ptr<FEBase> fe(FEBase::build(dim, FEType()));
  fe->get_phi();
  const std::vector<Point> & q_points_volume = fe->get_xyz();

  std::unique_ptr<FEBase> fe_face(FEBase::build(dim, FEType()));
  fe_face->get_phi();
  const std::vector<Point> & q_points_face = fe_face->get_xyz();

  fe->attach_quadrature_rule(&qrule);
  fe_face->attach_quadrature_rule(&qrule_face);

  // The current q_points
  const std::vector<Point> * q_points;

  if (parent_side != -1)
  {
    fe_face->reinit(elem, parent_side);
    q_points = &q_points_face;
  }
  else
  {
    fe->reinit(elem);
    q_points = &q_points_volume;
  }

  std::vector<Point> parent_ref_points;

  FEInterface::inverse_map(elem->dim(), FEType(), elem, *q_points, parent_ref_points);
  MeshRefinement mesh_refinement(mesh);
  mesh_refinement.uniformly_refine(1);

  std::map<unsigned int, std::vector<Point>> child_to_ref_points;

  unsigned int n_children = elem->n_children();

  refinement_map.resize(n_children);

  std::vector<unsigned int> children;

  if (child != -1) // Passed in a child explicitly
    children.push_back(child);
  else
  {
    children.resize(n_children);
    for (unsigned int child = 0; child < n_children; ++child)
      children[child] = child;
  }

  for (unsigned int i = 0; i < children.size(); ++i)
  {
    unsigned int child = children[i];

    if ((parent_side != -1 && !elem->is_child_on_side(child, parent_side)))
      continue;

    const Elem * child_elem = elem->child_ptr(child);

    if (child_side != -1)
    {
      fe_face->reinit(child_elem, child_side);
      q_points = &q_points_face;
    }
    else
    {
      fe->reinit(child_elem);
      q_points = &q_points_volume;
    }

    std::vector<Point> child_ref_points;

    FEInterface::inverse_map(elem->dim(), FEType(), elem, *q_points, child_ref_points);
    child_to_ref_points[child] = child_ref_points;

    std::vector<QpMap> & qp_map = refinement_map[child];

    // Find the closest parent_qp to each child_qp
    mapPoints(child_ref_points, parent_ref_points, qp_map);
  }

  coarsen_map.resize(parent_ref_points.size());

  // For each parent qp find the closest child qp
  for (unsigned int child = 0; child < n_children; child++)
  {
    if (parent_side != -1 && !elem->is_child_on_side(child, child_side))
      continue;

    std::vector<Point> & child_ref_points = child_to_ref_points[child];

    std::vector<QpMap> qp_map;

    // Find all of the closest points from parent_qp to _THIS_ child's qp
    mapPoints(parent_ref_points, child_ref_points, qp_map);

    // Check those to see if they are closer than what we currently have for each point
    for (unsigned int parent_qp = 0; parent_qp < parent_ref_points.size(); ++parent_qp)
    {
      std::pair<unsigned int, QpMap> & child_and_map = coarsen_map[parent_qp];
      unsigned int & closest_child = child_and_map.first;
      QpMap & closest_map = child_and_map.second;

      QpMap & current_map = qp_map[parent_qp];

      if (current_map._distance < closest_map._distance)
      {
        closest_child = child;
        closest_map = current_map;
      }
    }
  }
}

void
MooseMesh::changeBoundaryId(const boundary_id_type old_id,
                            const boundary_id_type new_id,
                            bool delete_prev)
{
  TIME_SECTION("changeBoundaryId", 6);
  changeBoundaryId(getMesh(), old_id, new_id, delete_prev);
}

void
MooseMesh::changeBoundaryId(MeshBase & mesh,
                            const boundary_id_type old_id,
                            const boundary_id_type new_id,
                            bool delete_prev)
{
  // Get a reference to our BoundaryInfo object, we will use it several times below...
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Container to catch ids passed back from BoundaryInfo
  std::vector<boundary_id_type> old_ids;

  // Only level-0 elements store BCs.  Loop over them.
  for (auto & elem : as_range(mesh.level_elements_begin(0), mesh.level_elements_end(0)))
  {
    unsigned int n_sides = elem->n_sides();
    for (unsigned int s = 0; s != n_sides; ++s)
    {
      boundary_info.boundary_ids(elem, s, old_ids);
      if (std::find(old_ids.begin(), old_ids.end(), old_id) != old_ids.end())
      {
        std::vector<boundary_id_type> new_ids(old_ids);
        std::replace(new_ids.begin(), new_ids.end(), old_id, new_id);
        if (delete_prev)
        {
          boundary_info.remove_side(elem, s);
          boundary_info.add_side(elem, s, new_ids);
        }
        else
          boundary_info.add_side(elem, s, new_ids);
      }
    }
  }

  // Remove any remaining references to the old ID from the
  // BoundaryInfo object.  This prevents things like empty sidesets
  // from showing up when printing information, etc.
  if (delete_prev)
    boundary_info.remove_id(old_id);
}

const RealVectorValue &
MooseMesh::getNormalByBoundaryID(BoundaryID id) const
{
  mooseAssert(_boundary_to_normal_map.get() != nullptr, "Boundary To Normal Map not built!");

  // Note: Boundaries that are not in the map (existing boundaries) will default
  // construct a new RealVectorValue - (x,y,z)=(0, 0, 0)
  return (*_boundary_to_normal_map)[id];
}

MooseMesh &
MooseMesh::clone() const
{
  mooseError("MooseMesh::clone() is no longer supported, use MooseMesh::safeClone() instead.");
}

void
MooseMesh::determineUseDistributedMesh()
{
  switch (_parallel_type)
  {
    case ParallelType::DEFAULT:
      // The user did not specify 'parallel_type = XYZ' in the input file,
      // so we allow the --distributed-mesh command line arg to possibly turn
      // on DistributedMesh.  If the command line arg is not present, we pick ReplicatedMesh.
      if (_app.getDistributedMeshOnCommandLine())
        _use_distributed_mesh = true;
      break;
    case ParallelType::REPLICATED:
      if (_app.getDistributedMeshOnCommandLine() || _is_nemesis || _is_split)
        _parallel_type_overridden = true;
      _use_distributed_mesh = false;
      break;
    case ParallelType::DISTRIBUTED:
      _use_distributed_mesh = true;
      break;
  }

  // If the user specifies 'nemesis = true' in the Mesh block, or they are using --use-split,
  // we must use DistributedMesh.
  if (_is_nemesis || _is_split)
    _use_distributed_mesh = true;
}

std::unique_ptr<MeshBase>
MooseMesh::buildMeshBaseObject(unsigned int dim)
{
  if (dim == libMesh::invalid_uint)
    dim = getParam<MooseEnum>("dim");

  std::unique_ptr<MeshBase> mesh;
  if (_use_distributed_mesh)
    mesh = buildTypedMesh<DistributedMesh>(dim);
  else
    mesh = buildTypedMesh<ReplicatedMesh>(dim);

  return mesh;
}

void
MooseMesh::setMeshBase(std::unique_ptr<MeshBase> mesh_base)
{
  _mesh = std::move(mesh_base);
  _mesh->allow_remote_element_removal(_allow_remote_element_removal);
}

void
MooseMesh::init()
{
  /**
   * If the mesh base hasn't been constructed by the time init is called, just do it here.
   * This can happen if somebody builds a mesh outside of the normal Action system. Forcing
   * developers to create, construct the MeshBase, and then init separately is a bit much for casual
   * use but it gives us the ability to run MeshGenerators in-between.
   */
  if (!_mesh)
    _mesh = buildMeshBaseObject();

  if (_app.isSplitMesh() && _use_distributed_mesh)
    mooseError("You cannot use the mesh splitter capability with DistributedMesh!");

  TIME_SECTION("init", 2);

  if (_app.isRecovering() && _allow_recovery && _app.isUltimateMaster())
  {
    // Some partitioners are not idempotent.  Some recovery data
    // files require partitioning to match mesh partitioning.  This
    // means that, when recovering, we can't safely repartition.
    const bool skip_partitioning_later = getMesh().skip_partitioning();
    getMesh().skip_partitioning(true);
    const bool allow_renumbering_later = getMesh().allow_renumbering();
    getMesh().allow_renumbering(false);

    // For now, only read the recovery mesh on the Ultimate Master..
    // sub-apps need to just build their mesh like normal
    {
      TIME_SECTION("readRecoveredMesh", 2);
      getMesh().read(_app.getRestartRecoverFileBase() + "_mesh." +
                     _app.getRestartRecoverFileSuffix());
    }

    getMesh().allow_renumbering(allow_renumbering_later);
    getMesh().skip_partitioning(skip_partitioning_later);
  }
  else // Normally just build the mesh
  {
    // Don't allow partitioning during building
    if (_app.isSplitMesh())
      getMesh().skip_partitioning(true);
    buildMesh();

    // Re-enable partitioning so the splitter can partition!
    if (_app.isSplitMesh())
      getMesh().skip_partitioning(false);

    if (getParam<bool>("build_all_side_lowerd_mesh"))
      buildLowerDMesh();
  }
}

unsigned int
MooseMesh::dimension() const
{
  return getMesh().mesh_dimension();
}

unsigned int
MooseMesh::effectiveSpatialDimension() const
{
  const Real abs_zero = 1e-12;

  // See if the mesh is completely containd in the z and y planes to calculate effective spatial
  // dim
  for (unsigned int dim = LIBMESH_DIM; dim >= 1; --dim)
    if (dimensionWidth(dim - 1) >= abs_zero)
      return dim;

  // If we get here, we have a 1D mesh on the x-axis.
  return 1;
}

std::vector<BoundaryID>
MooseMesh::getBoundaryIDs(const Elem * const elem, const unsigned short int side) const
{
  std::vector<BoundaryID> ids;
  getMesh().get_boundary_info().boundary_ids(elem, side, ids);
  return ids;
}

const std::set<BoundaryID> &
MooseMesh::getBoundaryIDs() const
{
  return getMesh().get_boundary_info().get_boundary_ids();
}

void
MooseMesh::buildNodeListFromSideList()
{
  if (_construct_node_list_from_side_list)
    getMesh().get_boundary_info().build_node_list_from_side_list();
}

void
MooseMesh::buildSideList(std::vector<dof_id_type> & el,
                         std::vector<unsigned short int> & sl,
                         std::vector<boundary_id_type> & il)
{
#ifdef LIBMESH_ENABLE_DEPRECATED
  mooseDeprecated("The version of MooseMesh::buildSideList() taking three arguments is "
                  "deprecated, call the version that returns a vector of tuples instead.");
  getMesh().get_boundary_info().build_side_list(el, sl, il);
#else
  libmesh_ignore(el);
  libmesh_ignore(sl);
  libmesh_ignore(il);
  mooseError("The version of MooseMesh::buildSideList() taking three "
             "arguments is not available in your version of libmesh, call the "
             "version that returns a vector of tuples instead.");
#endif
}

std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>>
MooseMesh::buildSideList()
{
  return getMesh().get_boundary_info().build_side_list();
}

std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>>
MooseMesh::buildActiveSideList() const
{
  return getMesh().get_boundary_info().build_active_side_list();
}

unsigned int
MooseMesh::sideWithBoundaryID(const Elem * const elem, const BoundaryID boundary_id) const
{
  return getMesh().get_boundary_info().side_with_boundary_id(elem, boundary_id);
}

MeshBase::node_iterator
MooseMesh::localNodesBegin()
{
  return getMesh().local_nodes_begin();
}

MeshBase::node_iterator
MooseMesh::localNodesEnd()
{
  return getMesh().local_nodes_end();
}

MeshBase::const_node_iterator
MooseMesh::localNodesBegin() const
{
  return getMesh().local_nodes_begin();
}

MeshBase::const_node_iterator
MooseMesh::localNodesEnd() const
{
  return getMesh().local_nodes_end();
}

MeshBase::element_iterator
MooseMesh::activeLocalElementsBegin()
{
  return getMesh().active_local_elements_begin();
}

const MeshBase::element_iterator
MooseMesh::activeLocalElementsEnd()
{
  return getMesh().active_local_elements_end();
}

MeshBase::const_element_iterator
MooseMesh::activeLocalElementsBegin() const
{
  return getMesh().active_local_elements_begin();
}

const MeshBase::const_element_iterator
MooseMesh::activeLocalElementsEnd() const
{
  return getMesh().active_local_elements_end();
}

dof_id_type
MooseMesh::nNodes() const
{
  return getMesh().n_nodes();
}

dof_id_type
MooseMesh::nElem() const
{
  return getMesh().n_elem();
}

dof_id_type
MooseMesh::maxNodeId() const
{
  return getMesh().max_node_id();
}

dof_id_type
MooseMesh::maxElemId() const
{
  return getMesh().max_elem_id();
}

Elem *
MooseMesh::elem(const dof_id_type i)
{
  mooseDeprecated("MooseMesh::elem() is deprecated, please use MooseMesh::elemPtr() instead");
  return elemPtr(i);
}

const Elem *
MooseMesh::elem(const dof_id_type i) const
{
  mooseDeprecated("MooseMesh::elem() is deprecated, please use MooseMesh::elemPtr() instead");
  return elemPtr(i);
}

Elem *
MooseMesh::elemPtr(const dof_id_type i)
{
  return getMesh().elem_ptr(i);
}

const Elem *
MooseMesh::elemPtr(const dof_id_type i) const
{
  return getMesh().elem_ptr(i);
}

Elem *
MooseMesh::queryElemPtr(const dof_id_type i)
{
  return getMesh().query_elem_ptr(i);
}

const Elem *
MooseMesh::queryElemPtr(const dof_id_type i) const
{
  return getMesh().query_elem_ptr(i);
}

bool
MooseMesh::prepared() const
{
  return _mesh->is_prepared() && _moose_mesh_prepared;
}

void
MooseMesh::prepared(bool state)
{
  if (state)
    mooseError("We don't have any right to tell the libmesh mesh that it *is* prepared. Only a "
               "call to prepare_for_use should tell us that");

  // Some people may call this even before we have a MeshBase object. This isn't dangerous really
  // because when the MeshBase object is born, it knows it's in an unprepared state
  if (_mesh)
    _mesh->set_isnt_prepared();

  // If the libMesh mesh isn't preparead, then our MooseMesh wrapper is also no longer prepared
  _moose_mesh_prepared = false;

  /**
   * If we are explicitly setting the mesh to not prepared, then we've likely modified the mesh
   * and can no longer make assumptions about orthogonality. We really should recheck.
   */
  _regular_orthogonal_mesh = false;
}

void
MooseMesh::needsPrepareForUse()
{
  prepared(false);
}

const std::set<SubdomainID> &
MooseMesh::meshSubdomains() const
{
  return _mesh_subdomains;
}

const std::set<BoundaryID> &
MooseMesh::meshBoundaryIds() const
{
  return _mesh_boundary_ids;
}

const std::set<BoundaryID> &
MooseMesh::meshSidesetIds() const
{
  return _mesh_sideset_ids;
}

const std::set<BoundaryID> &
MooseMesh::meshNodesetIds() const
{
  return _mesh_nodeset_ids;
}

void
MooseMesh::setMeshBoundaryIDs(std::set<BoundaryID> boundary_IDs)
{
  _mesh_boundary_ids = boundary_IDs;
}

void
MooseMesh::setBoundaryToNormalMap(
    std::unique_ptr<std::map<BoundaryID, RealVectorValue>> boundary_map)
{
  _boundary_to_normal_map = std::move(boundary_map);
}

void
MooseMesh::setBoundaryToNormalMap(std::map<BoundaryID, RealVectorValue> * boundary_map)
{
  mooseDeprecated("setBoundaryToNormalMap(std::map<BoundaryID, RealVectorValue> * boundary_map) is "
                  "deprecated, use the unique_ptr version instead");
  _boundary_to_normal_map.reset(boundary_map);
}

unsigned int
MooseMesh::uniformRefineLevel() const
{
  return _uniform_refine_level;
}

void
MooseMesh::setUniformRefineLevel(unsigned int level, bool deletion)
{
  _uniform_refine_level = level;
  _skip_deletion_repartition_after_refine = deletion;
}

void
MooseMesh::addGhostedBoundary(BoundaryID boundary_id)
{
  _ghosted_boundaries.insert(boundary_id);
}

void
MooseMesh::setGhostedBoundaryInflation(const std::vector<Real> & inflation)
{
  _ghosted_boundaries_inflation = inflation;
}

const std::set<unsigned int> &
MooseMesh::getGhostedBoundaries() const
{
  return _ghosted_boundaries;
}

const std::vector<Real> &
MooseMesh::getGhostedBoundaryInflation() const
{
  return _ghosted_boundaries_inflation;
}

namespace // Anonymous namespace for helpers
{
// A class for templated methods that expect output iterator
// arguments, which adds objects to the Mesh.
// Although any mesh_inserter_iterator can add any object, we
// template it around object type so that type inference and
// iterator_traits will work.
// This object specifically is used to insert extra ghost elems into the mesh
template <typename T>
struct extra_ghost_elem_inserter
{
  using iterator_category = std::output_iterator_tag;
  using value_type = T;

  extra_ghost_elem_inserter(DistributedMesh & m) : mesh(m) {}

  void operator=(const Elem * e) { mesh.add_extra_ghost_elem(const_cast<Elem *>(e)); }

  void operator=(Node * n) { mesh.add_node(n); }

  void operator=(Point * p) { mesh.add_point(*p); }

  extra_ghost_elem_inserter & operator++() { return *this; }

  extra_ghost_elem_inserter operator++(int) { return extra_ghost_elem_inserter(*this); }

  // We don't return a reference-to-T here because we don't want to
  // construct one or have any of its methods called.  We just want
  // to allow the returned object to be able to do mesh insertions
  // with operator=().
  extra_ghost_elem_inserter & operator*() { return *this; }

private:
  DistributedMesh & mesh;
};

/**
 * Specific weak ordering for Elem *'s to be used in a set.
 * We use the id, but first sort by level.  This guarantees
 * when traversing the set from beginning to end the lower
 * level (parent) elements are encountered first.
 *
 * This was swiped from libMesh mesh_communication.C, and ought to be
 * replaced with libMesh::CompareElemIdsByLevel just as soon as I refactor to
 * create that - @roystgnr
 */
struct CompareElemsByLevel
{
  bool operator()(const Elem * a, const Elem * b) const
  {
    libmesh_assert(a);
    libmesh_assert(b);
    const unsigned int al = a->level(), bl = b->level();
    const dof_id_type aid = a->id(), bid = b->id();

    return (al == bl) ? aid < bid : al < bl;
  }
};

} // anonymous namespace

void
MooseMesh::ghostGhostedBoundaries()
{
  // No need to do this if using a serial mesh
  // We do not need to ghost boundary elements when _need_ghost_ghosted_boundaries
  // is not true. _need_ghost_ghosted_boundaries can be set by a mesh generator
  // where boundaries are already ghosted accordingly
  if (!_use_distributed_mesh || !_need_ghost_ghosted_boundaries)
    return;

  TIME_SECTION("GhostGhostedBoundaries", 3);

  DistributedMesh & mesh = dynamic_cast<DistributedMesh &>(getMesh());

  // We clear ghosted elements that were added by previous invocations of this
  // method but leave ghosted elements that were added by other code, e.g.
  // OversampleOutput, untouched
  mesh.clear_extra_ghost_elems(_ghost_elems_from_ghost_boundaries);
  _ghost_elems_from_ghost_boundaries.clear();

  std::set<const Elem *, CompareElemsByLevel> boundary_elems_to_ghost;
  std::set<Node *> connected_nodes_to_ghost;

  std::vector<const Elem *> family_tree;

  for (const auto & t : mesh.get_boundary_info().build_side_list())
  {
    auto elem_id = std::get<0>(t);
    auto bc_id = std::get<2>(t);

    if (_ghosted_boundaries.find(bc_id) != _ghosted_boundaries.end())
    {
      Elem * elem = mesh.elem_ptr(elem_id);

#ifdef LIBMESH_ENABLE_AMR
      elem->family_tree(family_tree);
      Elem * parent = elem->parent();
      while (parent)
      {
        family_tree.push_back(parent);
        parent = parent->parent();
      }
#else
      family_tree.clear();
      family_tree.push_back(elem);
#endif
      for (const auto & felem : family_tree)
      {
        boundary_elems_to_ghost.insert(felem);

        // The entries of connected_nodes_to_ghost need to be
        // non-constant, so that they will work in things like
        // UpdateDisplacedMeshThread. The container returned by
        // family_tree contains const Elems even when the Elem
        // it is called on is non-const, so once that interface
        // gets fixed we can remove this const_cast.
        for (unsigned int n = 0; n < felem->n_nodes(); ++n)
          connected_nodes_to_ghost.insert(const_cast<Node *>(felem->node_ptr(n)));
      }
    }
  }

  // We really do want to store this by value instead of by reference
  const auto prior_ghost_elems = mesh.extra_ghost_elems();

  mesh.comm().allgather_packed_range(&mesh,
                                     connected_nodes_to_ghost.begin(),
                                     connected_nodes_to_ghost.end(),
                                     extra_ghost_elem_inserter<Node>(mesh));

  mesh.comm().allgather_packed_range(&mesh,
                                     boundary_elems_to_ghost.begin(),
                                     boundary_elems_to_ghost.end(),
                                     extra_ghost_elem_inserter<Elem>(mesh));

  const auto & current_ghost_elems = mesh.extra_ghost_elems();

  std::set_difference(current_ghost_elems.begin(),
                      current_ghost_elems.end(),
                      prior_ghost_elems.begin(),
                      prior_ghost_elems.end(),
                      std::inserter(_ghost_elems_from_ghost_boundaries,
                                    _ghost_elems_from_ghost_boundaries.begin()));
}

unsigned int
MooseMesh::getPatchSize() const
{
  return _patch_size;
}

void
MooseMesh::setPatchUpdateStrategy(Moose::PatchUpdateType patch_update_strategy)
{
  _patch_update_strategy = patch_update_strategy;
}

const Moose::PatchUpdateType &
MooseMesh::getPatchUpdateStrategy() const
{
  return _patch_update_strategy;
}

BoundingBox
MooseMesh::getInflatedProcessorBoundingBox(Real inflation_multiplier) const
{
  // Grab a bounding box to speed things up.  Note that
  // local_bounding_box is *not* equivalent to processor_bounding_box
  // with processor_id() except in serial.
  BoundingBox bbox = MeshTools::create_local_bounding_box(getMesh());

  // Inflate the bbox just a bit to deal with roundoff
  // Adding 1% of the diagonal size in each direction on each end
  Real inflation_amount = inflation_multiplier * (bbox.max() - bbox.min()).norm();
  Point inflation(inflation_amount, inflation_amount, inflation_amount);

  bbox.first -= inflation;  // min
  bbox.second += inflation; // max

  return bbox;
}

MooseMesh::operator libMesh::MeshBase &() { return getMesh(); }

MooseMesh::operator const libMesh::MeshBase &() const { return getMesh(); }

const MeshBase *
MooseMesh::getMeshPtr() const
{
  return _mesh.get();
}

MeshBase &
MooseMesh::getMesh()
{
  mooseAssert(_mesh, "Mesh hasn't been created");
  return *_mesh;
}

const MeshBase &
MooseMesh::getMesh() const
{
  mooseAssert(_mesh, "Mesh hasn't been created");
  return *_mesh;
}

void
MooseMesh::printInfo(std::ostream & os, const unsigned int verbosity /* = 0 */) const
{
  os << '\n';
  getMesh().print_info(os, verbosity);
  os << std::flush;
}

const std::vector<dof_id_type> &
MooseMesh::getNodeList(boundary_id_type nodeset_id) const
{
  std::map<boundary_id_type, std::vector<dof_id_type>>::const_iterator it =
      _node_set_nodes.find(nodeset_id);

  if (it == _node_set_nodes.end())
  {
    // On a distributed mesh we might not know about a remote nodeset,
    // so we'll return an empty vector and hope the nodeset exists
    // elsewhere.
    if (!getMesh().is_serial())
    {
      static const std::vector<dof_id_type> empty_vec;
      return empty_vec;
    }
    // On a replicated mesh we should know about every nodeset and if
    // we're asked for one that doesn't exist then it must be a bug.
    else
    {
      mooseError("Unable to nodeset ID: ", nodeset_id, '.');
    }
  }

  return it->second;
}

const std::set<BoundaryID> &
MooseMesh::getSubdomainBoundaryIds(const SubdomainID subdomain_id) const
{
  std::unordered_map<SubdomainID, std::set<BoundaryID>>::const_iterator it =
      _subdomain_boundary_ids.find(subdomain_id);

  if (it == _subdomain_boundary_ids.end())
    mooseError("Unable to find subdomain ID: ", subdomain_id, '.');

  return it->second;
}

std::set<BoundaryID>
MooseMesh::getSubdomainInterfaceBoundaryIds(const SubdomainID subdomain_id) const
{
  const auto & bnd_ids = getSubdomainBoundaryIds(subdomain_id);
  std::set<BoundaryID> boundary_ids(bnd_ids.begin(), bnd_ids.end());
  std::unordered_map<SubdomainID, std::set<BoundaryID>>::const_iterator it =
      _neighbor_subdomain_boundary_ids.find(subdomain_id);

  boundary_ids.insert(it->second.begin(), it->second.end());

  return boundary_ids;
}

std::set<SubdomainID>
MooseMesh::getBoundaryConnectedBlocks(const BoundaryID bid) const
{
  std::set<SubdomainID> subdomain_ids;
  for (const auto & it : _subdomain_boundary_ids)
    if (it.second.find(bid) != it.second.end())
      subdomain_ids.insert(it.first);

  return subdomain_ids;
}

std::set<SubdomainID>
MooseMesh::getBoundaryConnectedSecondaryBlocks(const BoundaryID bid) const
{
  std::set<SubdomainID> subdomain_ids;
  for (const auto & it : _neighbor_subdomain_boundary_ids)
    if (it.second.find(bid) != it.second.end())
      subdomain_ids.insert(it.first);

  return subdomain_ids;
}

std::set<SubdomainID>
MooseMesh::getInterfaceConnectedBlocks(const BoundaryID bid) const
{
  std::set<SubdomainID> subdomain_ids = getBoundaryConnectedBlocks(bid);
  for (const auto & it : _neighbor_subdomain_boundary_ids)
    if (it.second.find(bid) != it.second.end())
      subdomain_ids.insert(it.first);

  return subdomain_ids;
}

const std::set<SubdomainID> &
MooseMesh::getBlockConnectedBlocks(const SubdomainID subdomain_id) const
{
  auto it = _sub_to_neighbor_subs.find(subdomain_id);

  if (it == _sub_to_neighbor_subs.end())
    mooseError("Unable to find subdomain ID: ", subdomain_id, '.');

  return it->second;
}

bool
MooseMesh::isBoundaryNode(dof_id_type node_id) const
{
  bool found_node = false;
  for (const auto & it : _bnd_node_ids)
  {
    if (it.second.find(node_id) != it.second.end())
    {
      found_node = true;
      break;
    }
  }
  return found_node;
}

bool
MooseMesh::isBoundaryNode(dof_id_type node_id, BoundaryID bnd_id) const
{
  bool found_node = false;
  std::map<boundary_id_type, std::set<dof_id_type>>::const_iterator it = _bnd_node_ids.find(bnd_id);
  if (it != _bnd_node_ids.end())
    if (it->second.find(node_id) != it->second.end())
      found_node = true;
  return found_node;
}

bool
MooseMesh::isBoundaryElem(dof_id_type elem_id) const
{
  bool found_elem = false;
  for (const auto & it : _bnd_elem_ids)
  {
    if (it.second.find(elem_id) != it.second.end())
    {
      found_elem = true;
      break;
    }
  }
  return found_elem;
}

bool
MooseMesh::isBoundaryElem(dof_id_type elem_id, BoundaryID bnd_id) const
{
  bool found_elem = false;
  auto it = _bnd_elem_ids.find(bnd_id);
  if (it != _bnd_elem_ids.end())
    if (it->second.find(elem_id) != it->second.end())
      found_elem = true;
  return found_elem;
}

void
MooseMesh::errorIfDistributedMesh(std::string name) const
{
  if (_use_distributed_mesh)
    mooseError("Cannot use ",
               name,
               " with DistributedMesh!\n",
               "Consider specifying parallel_type = 'replicated' in your input file\n",
               "to prevent it from being run with DistributedMesh.");
}

void
MooseMesh::setPartitionerHelper(MeshBase * const mesh)
{
  if (_use_distributed_mesh && (_partitioner_name != "default" && _partitioner_name != "parmetis"))
  {
    _partitioner_name = "parmetis";
    _partitioner_overridden = true;
  }

  setPartitioner(mesh ? *mesh : getMesh(), _partitioner_name, _use_distributed_mesh, _pars, *this);
}

void
MooseMesh::setPartitioner(MeshBase & mesh_base,
                          MooseEnum & partitioner,
                          bool use_distributed_mesh,
                          const InputParameters & params,
                          MooseObject & context_obj)
{
  // Set the partitioner based on partitioner name
  switch (partitioner)
  {
    case -3: // default
      // We'll use the default partitioner, but notify the user of which one is being used...
      if (use_distributed_mesh)
        partitioner = "parmetis";
      else
        partitioner = "metis";
      break;

    // No need to explicitily create the metis or parmetis partitioners,
    // They are the default for serial and parallel mesh respectively
    case -2: // metis
    case -1: // parmetis
      break;

    case 0: // linear
      mesh_base.partitioner().reset(new LinearPartitioner);
      break;
    case 1: // centroid
    {
      if (!params.isParamValid("centroid_partitioner_direction"))
        context_obj.paramError(
            "centroid_partitioner_direction",
            "If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

      MooseEnum direction = params.get<MooseEnum>("centroid_partitioner_direction");

      if (direction == "x")
        mesh_base.partitioner().reset(new CentroidPartitioner(CentroidPartitioner::X));
      else if (direction == "y")
        mesh_base.partitioner().reset(new CentroidPartitioner(CentroidPartitioner::Y));
      else if (direction == "z")
        mesh_base.partitioner().reset(new CentroidPartitioner(CentroidPartitioner::Z));
      else if (direction == "radial")
        mesh_base.partitioner().reset(new CentroidPartitioner(CentroidPartitioner::RADIAL));
      break;
    }
    case 2: // hilbert_sfc
      mesh_base.partitioner().reset(new HilbertSFCPartitioner);
      break;
    case 3: // morton_sfc
      mesh_base.partitioner().reset(new MortonSFCPartitioner);
      break;
  }
}

void
MooseMesh::setCustomPartitioner(Partitioner * partitioner)
{
  _custom_partitioner = partitioner->clone();
}

bool
MooseMesh::isCustomPartitionerRequested() const
{
  return _custom_partitioner_requested;
}

bool
MooseMesh::hasSecondOrderElements()
{
  bool mesh_has_second_order_elements = false;
  for (auto it = activeLocalElementsBegin(), end = activeLocalElementsEnd(); it != end; ++it)
    if ((*it)->default_order() == SECOND)
    {
      mesh_has_second_order_elements = true;
      break;
    }

  // We checked our local elements, so take the max over all processors.
  comm().max(mesh_has_second_order_elements);
  return mesh_has_second_order_elements;
}

void
MooseMesh::setIsCustomPartitionerRequested(bool cpr)
{
  _custom_partitioner_requested = cpr;
}

std::unique_ptr<PointLocatorBase>
MooseMesh::getPointLocator() const
{
  return getMesh().sub_point_locator();
}

void
MooseMesh::buildFiniteVolumeInfo() const
{
  if (!_finite_volume_info_dirty)
    return;

  mooseAssert(!Threads::in_threads,
              "This routine has not been implemented for threads. Please query this routine before "
              "a threaded region or contact a MOOSE developer to discuss.");
  _finite_volume_info_dirty = false;

  using Keytype = std::pair<const Elem *, unsigned short int>;

  // create a map from elem/side --> boundary ids
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> side_list =
      buildActiveSideList();
  std::map<Keytype, std::set<boundary_id_type>> side_map;
  for (auto & e : side_list)
  {
    const Elem * elem = _mesh->elem_ptr(std::get<0>(e));
    Keytype key(elem, std::get<1>(e));
    auto it = side_map.find(key);
    if (it == side_map.end())
      side_map[key] = {std::get<2>(e)};
    else
      side_map[key].insert(std::get<2>(e));
  }

  _face_info.clear();
  _all_face_info.clear();
  _elem_side_to_face_info.clear();

  _elem_to_elem_info.clear();

  // by performing the element ID comparison check in the below loop, we are ensuring that we never
  // double count face contributions. If a face lies along a process boundary, the only process that
  // will contribute to both sides of the face residuals/Jacobians will be the process that owns the
  // element with the lower ID.
  auto begin = getMesh().active_elements_begin();
  auto end = getMesh().active_elements_end();

  // We prepare a map connecting the Elem* and the corresponding ElemInfo
  // for the active elements.
  for (const Elem * elem : as_range(begin, end))
    _elem_to_elem_info.emplace(elem->id(), elem);

  dof_id_type face_index = 0;
  for (const Elem * elem : as_range(begin, end))
  {
    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      // get the neighbor element
      const Elem * neighbor = elem->neighbor_ptr(side);

      // Check if the FaceInfo shall belong to the element. If yes,
      // create and initialize the FaceInfo. We need this to ensure that
      // we do not duplicate FaceInfo-s.
      if (Moose::FV::elemHasFaceInfo(*elem, neighbor))
      {
        mooseAssert(!neighbor || (neighbor->level() < elem->level() ? neighbor->active() : true),
                    "If the neighbor is coarser than the element, we expect that the neighbor must "
                    "be active.");

        // We construct the faceInfo using the elementinfo and side index
        _all_face_info.emplace_back(&_elem_to_elem_info[elem->id()], side, face_index++);

        auto & fi = _all_face_info.back();

        // get all the sidesets that this face is contained in and cache them
        // in the face info.
        std::set<boundary_id_type> & boundary_ids = fi.boundaryIDs();
        boundary_ids.clear();

        // We initialize the weights/other information in faceInfo. If the neighbor does not exist
        // or is remote (so when we are on some sort of mesh boundary), we initiualize the ghost
        // cell and use it to compute the weights corresponding to the faceInfo.
        if (!neighbor || neighbor == remote_elem)
          fi.computeBoundaryCoefficients();
        else
          fi.computeInternalCoefficients(&_elem_to_elem_info[neighbor->id()]);

        auto lit = side_map.find(Keytype(&fi.elem(), fi.elemSideID()));
        if (lit != side_map.end())
          boundary_ids.insert(lit->second.begin(), lit->second.end());

        if (fi.neighborPtr())
        {
          auto rit = side_map.find(Keytype(fi.neighborPtr(), fi.neighborSideID()));
          if (rit != side_map.end())
            boundary_ids.insert(rit->second.begin(), rit->second.end());
        }
      }
    }
  }

  // Build the local face info and elem_side to face info maps. We need to do this after
  // _all_face_info is finished being constructed because emplace_back invalidates all iterators and
  // references if ever the new size exceeds capacity
  for (auto & fi : _all_face_info)
  {
    const Elem * const elem = &fi.elem();
    const auto side = fi.elemSideID();

#ifndef NDEBUG
    auto pair_it =
#endif
        _elem_side_to_face_info.emplace(std::make_pair(elem, side), &fi);
    mooseAssert(pair_it.second, "We should be adding unique FaceInfo objects.");

    // We will add the faces on processor boundaries to the list of face infos on each
    // associated processor.
    if (fi.elem().processor_id() == this->processor_id() ||
        (fi.neighborPtr() && (fi.neighborPtr()->processor_id() == this->processor_id())))
      _face_info.push_back(&fi);
  }
}

const FaceInfo *
MooseMesh::faceInfo(const Elem * elem, unsigned int side) const
{
  buildFiniteVolumeInfo();

  auto it = _elem_side_to_face_info.find(std::make_pair(elem, side));

  if (it == _elem_side_to_face_info.end())
    return nullptr;
  else
  {
    mooseAssert(it->second, "For some reason, the FaceInfo object is NULL!");
    return it->second;
  }
}

const ElemInfo &
MooseMesh::elemInfo(const dof_id_type id) const
{
  return libmesh_map_find(_elem_to_elem_info, id);
}

void
MooseMesh::computeFaceInfoFaceCoords()
{
  if (_finite_volume_info_dirty)
    mooseError("Trying to compute face-info coords when the information is dirty");

  for (auto & fi : _all_face_info)
  {
    // get elem & neighbor elements, and set subdomain ids
    const SubdomainID elem_subdomain_id = fi.elemSubdomainID();
    const SubdomainID neighbor_subdomain_id = fi.neighborSubdomainID();

    coordTransformFactor(
        *this, elem_subdomain_id, fi.faceCentroid(), fi.faceCoord(), neighbor_subdomain_id);
  }
}

MooseEnum
MooseMesh::partitioning()
{
  MooseEnum partitioning("default=-3 metis=-2 parmetis=-1 linear=0 centroid hilbert_sfc morton_sfc",
                         "default");
  return partitioning;
}

void
MooseMesh::allowRemoteElementRemoval(const bool allow_remote_element_removal)
{
  _allow_remote_element_removal = allow_remote_element_removal;
  if (_mesh)
    _mesh->allow_remote_element_removal(allow_remote_element_removal);

  if (!allow_remote_element_removal)
    // If we're not allowing remote element removal now, then we will need deletion later after
    // late geoemetric ghosting functors have been added (late geometric ghosting functor addition
    // happens when algebraic ghosting functors are added)
    _need_delete = true;
}

void
MooseMesh::deleteRemoteElements()
{
  _allow_remote_element_removal = true;
  if (!_mesh)
    mooseError("Cannot delete remote elements because we have not yet attached a MeshBase");

  _mesh->allow_remote_element_removal(true);

  _mesh->delete_remote_elements();
}

void
MooseMesh::cacheVarIndicesByFace(const std::vector<const MooseVariableFieldBase *> & moose_vars)
{
  buildFiniteVolumeInfo();

  for (FaceInfo & face : _all_face_info)
  {
    const SubdomainID elem_subdomain_id = face.elemSubdomainID();
    const SubdomainID neighbor_subdomain_id = face.neighborSubdomainID();

    // loop through vars
    for (unsigned int j = 0; j < moose_vars.size(); ++j)
    {
      // get the variable, its name, and its domain of definition
      const MooseVariableFieldBase * const var = moose_vars[j];
      const auto & var_name = var->name();
      std::set<SubdomainID> var_subdomains = var->blockIDs();

      // unfortunately, MOOSE is lazy and all subdomains has its own
      // ID. If ANY_BLOCK_ID is in var_subdomains, inject all subdomains explicitly
      if (var_subdomains.find(Moose::ANY_BLOCK_ID) != var_subdomains.end())
        var_subdomains = this->meshSubdomains();

      /**
       * The following paragraph of code assigns the VarFaceNeighbors
       * 1. The face is an internal face of this variable if it is defined on
       *    the elem and neighbor subdomains
       * 2. The face is an invalid face of this variable if it is neither defined
       *    on the elem nor the neighbor subdomains
       * 3. If not 1. or 2. then this is a boundary for this variable and the else clause
       *    applies
       */
      bool var_defined_elem = var_subdomains.find(elem_subdomain_id) != var_subdomains.end();
      bool var_defined_neighbor =
          var_subdomains.find(neighbor_subdomain_id) != var_subdomains.end();
      if (var_defined_elem && var_defined_neighbor)
        face.faceType(var_name) = FaceInfo::VarFaceNeighbors::BOTH;
      else if (!var_defined_elem && !var_defined_neighbor)
        face.faceType(var_name) = FaceInfo::VarFaceNeighbors::NEITHER;
      else
      {
        // this is a boundary face for this variable, set elem or neighbor
        if (var_defined_elem)
          face.faceType(var_name) = FaceInfo::VarFaceNeighbors::ELEM;
        else if (var_defined_neighbor)
          face.faceType(var_name) = FaceInfo::VarFaceNeighbors::NEIGHBOR;
        else
          mooseError("Should never get here");
      }
    }
  }
}

void
MooseMesh::setCoordSystem(const std::vector<SubdomainName> & blocks,
                          const MultiMooseEnum & coord_sys)
{
  TIME_SECTION("setCoordSystem", 5, "Setting Coordinate System");
  if (!_provided_coord_blocks.empty() && (_provided_coord_blocks != blocks))
  {
    const std::string param_name = isParamValid("coord_block") ? "coord_block" : "block";
    mooseWarning("Supplied blocks in the 'setCoordSystem' method do not match the value of the "
                 "'Mesh/",
                 param_name,
                 "' parameter. Did you provide different parameter values for 'Mesh/",
                 param_name,
                 "' and 'Problem/block'?. We will honor the parameter value from 'Mesh/",
                 param_name,
                 "'");
    mooseAssert(_coord_system_set,
                "If we are arriving here due to a bad specification in the Problem block, then we "
                "should have already set our coordinate system subdomains from the Mesh block");
    return;
  }
  if (_pars.isParamSetByUser("coord_type") && getParam<MultiMooseEnum>("coord_type") != coord_sys)
    mooseError("Supplied coordinate systems in the 'setCoordSystem' method do not match the value "
               "of the 'Mesh/coord_type' parameter. Did you provide different parameter values for "
               "'coord_type' to 'Mesh' and 'Problem'?");

  auto subdomains = meshSubdomains();
  // It's possible that a user has called this API before the mesh is prepared and consequently we
  // don't yet have the subdomains in meshSubdomains()
  for (const auto & sub_name : blocks)
  {
    const auto sub_id = getSubdomainID(sub_name);
    subdomains.insert(sub_id);
  }

  if (coord_sys.size() <= 1)
  {
    // We will specify the same coordinate system for all blocks
    const auto coord_type = coord_sys.size() == 0
                                ? Moose::COORD_XYZ
                                : Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[0]);
    for (const auto sid : subdomains)
      _coord_sys[sid] = coord_type;
  }
  else
  {
    if (blocks.size() != coord_sys.size())
      mooseError("Number of blocks and coordinate systems does not match.");

    for (const auto i : index_range(blocks))
    {
      SubdomainID sid = getSubdomainID(blocks[i]);
      Moose::CoordinateSystemType coord_type =
          Moose::stringToEnum<Moose::CoordinateSystemType>(coord_sys[i]);
      _coord_sys[sid] = coord_type;
    }

    for (const auto & sid : subdomains)
      if (_coord_sys.find(sid) == _coord_sys.end())
        mooseError("Subdomain '" + Moose::stringify(sid) +
                   "' does not have a coordinate system specified.");
  }

  _coord_system_set = true;

  updateCoordTransform();
}

Moose::CoordinateSystemType
MooseMesh::getCoordSystem(SubdomainID sid) const
{
  auto it = _coord_sys.find(sid);
  if (it != _coord_sys.end())
    return (*it).second;
  else
    mooseError("Requested subdomain ", sid, " does not exist.");
}

Moose::CoordinateSystemType
MooseMesh::getUniqueCoordSystem() const
{
  const auto unique_system = _coord_sys.find(*meshSubdomains().begin())->second;
  // Check that it is actually unique
  bool result = std::all_of(
      std::next(_coord_sys.begin()),
      _coord_sys.end(),
      [unique_system](
          typename std::unordered_map<SubdomainID, Moose::CoordinateSystemType>::const_reference
              item) { return (item.second == unique_system); });
  if (!result)
    mooseError("The unique coordinate system of the mesh was requested by the mesh contains "
               "multiple blocks with different coordinate systems");
  return unique_system;
}

const std::map<SubdomainID, Moose::CoordinateSystemType> &
MooseMesh::getCoordSystem() const
{
  return _coord_sys;
}

void
MooseMesh::setAxisymmetricCoordAxis(const MooseEnum & rz_coord_axis)
{
  _rz_coord_axis = rz_coord_axis;

  updateCoordTransform();
}

void
MooseMesh::updateCoordTransform()
{
  if (!_coord_transform)
    _coord_transform = std::make_unique<MooseAppCoordTransform>(*this);
  else
    _coord_transform->setCoordinateSystem(*this);
}

unsigned int
MooseMesh::getAxisymmetricRadialCoord() const
{
  if (_rz_coord_axis == 0)
    return 1; // if the rotation axis is x (0), then the radial direction is y (1)
  else
    return 0; // otherwise the radial direction is assumed to be x, i.e., the rotation axis is y
}

void
MooseMesh::checkCoordinateSystems()
{
  for (const auto & elem : getMesh().element_ptr_range())
  {
    SubdomainID sid = elem->subdomain_id();
    if (_coord_sys[sid] == Moose::COORD_RZ && elem->dim() == 3)
      mooseError("An RZ coordinate system was requested for subdomain " + Moose::stringify(sid) +
                 " which contains 3D elements.");
    if (_coord_sys[sid] == Moose::COORD_RSPHERICAL && elem->dim() > 1)
      mooseError("An RSPHERICAL coordinate system was requested for subdomain " +
                 Moose::stringify(sid) + " which contains 2D or 3D elements.");
  }
}

void
MooseMesh::setCoordData(const MooseMesh & other_mesh)
{
  _coord_sys = other_mesh._coord_sys;
  _rz_coord_axis = other_mesh._rz_coord_axis;
}

const MooseUnits &
MooseMesh::lengthUnit() const
{
  mooseAssert(_coord_transform, "This must be non-null");
  return _coord_transform->lengthUnit();
}

void
MooseMesh::checkDuplicateSubdomainNames()
{
  std::map<SubdomainName, SubdomainID> subdomain;
  for (const auto & sbd_id : _mesh_subdomains)
  {
    std::string sub_name = getSubdomainName(sbd_id);
    if (!sub_name.empty() && subdomain.count(sub_name) > 0)
      mooseError("The subdomain name ",
                 sub_name,
                 " is used for both subdomain with ID=",
                 subdomain[sub_name],
                 " and ID=",
                 sbd_id,
                 ", Please rename one of them!");
    else
      subdomain[sub_name] = sbd_id;
  }
}
