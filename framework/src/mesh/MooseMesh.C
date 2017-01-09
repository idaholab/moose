/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseMesh.h"
#include "Factory.h"
#include "CacheChangedListsThread.h"
#include "Assembly.h"
#include "MooseUtils.h"
#include "MooseApp.h"

#include <utility>

// libMesh
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/fe_interface.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_inserter_iterator.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_inserter_iterator.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_mesh.h"
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


static const int GRAIN_SIZE = 1;     // the grain_size does not have much influence on our execution speed

template<>
InputParameters validParams<MooseMesh>()
{
  InputParameters params = validParams<MooseObject>();

  MooseEnum mesh_distribution_type("PARALLEL=0 SERIAL DEFAULT", "DEFAULT");
  params.addParam<MooseEnum>("distribution", mesh_distribution_type,
                             "PARALLEL: Always use libMesh::DistributedMesh "
                             "SERIAL: Always use libMesh::ReplicatedMesh "
                             "DEFAULT: Use libMesh::ReplicatedMesh unless --distributed-mesh is specified on the command line "
                             "The distribution flag is deprecated, use parallel_type={DISTRIBUTED,REPLICATED} instead.");

  MooseEnum mesh_parallel_type("DISTRIBUTED=0 REPLICATED DEFAULT", "DEFAULT");
  params.addParam<MooseEnum>("parallel_type", mesh_parallel_type,
                             "DISTRIBUTED: Always use libMesh::DistributedMesh "
                             "REPLICATED: Always use libMesh::ReplicatedMesh "
                             "DEFAULT: Use libMesh::ReplicatedMesh unless --distributed-mesh is specified on the command line");

  params.addParam<bool>("nemesis", false,
                        "If nemesis=true and file=foo.e, actually reads "
                        "foo.e.N.0, foo.e.N.1, ... foo.e.N.N-1, "
                        "where N = # CPUs, with NemesisIO.");

  MooseEnum dims("1=1 2 3", "3");
  params.addParam<MooseEnum>("dim", dims,
                             "This is only required for certain mesh formats where "
                             "the dimension of the mesh cannot be autodetected.  "
                             "In particular you must supply this for GMSH meshes.  "
                             "Note: This is completely ignored for ExodusII meshes!");

  MooseEnum partitioning("default=-3 metis=-2 parmetis=-1 linear=0 centroid hilbert_sfc morton_sfc", "default");
  params.addParam<MooseEnum>("partitioner", partitioning, "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation.");
  MooseEnum direction("x y z radial");
  params.addParam<MooseEnum>("centroid_partitioner_direction", direction, "Specifies the sort direction if using the centroid partitioner. Available options: x, y, z, radial");

  MooseEnum patch_update_strategy("never always auto", "never");
  params.addParam<MooseEnum>("patch_update_strategy", patch_update_strategy,  "How often to update the geometric search 'patch'.  The default is to never update it (which is the most efficient but could be a problem with lots of relative motion).  'always' will update the patch every timestep which might be time consuming.  'auto' will attempt to determine when the patch size needs to be updated automatically.");

  // Note: This parameter is named to match 'construct_side_list_from_node_list' in SetupMeshAction
  params.addParam<bool>("construct_node_list_from_side_list", true, "Whether or not to generate nodesets from the sidesets (usually a good idea).");
  params.addParam<unsigned short>("num_ghosted_layers", 1, "Parameter to specify the number of geometric element layers"
                                  " that will be available when DistributedMesh is used. Value is ignored in ReplicatedMesh mode");
  params.addParam<bool>("ghost_point_neighbors", false, "Boolean to specify whether or not all point neighbors are ghosted"
                        " when DistributedMesh is used. Value is ignored in ReplicatedMesh mode");

  params.registerBase("MooseMesh");

  // groups
  params.addParamNamesToGroup("dim nemesis patch_update_strategy construct_node_list_from_side_list num_ghosted_layers"
                              " ghost_point_neighbors", "Advanced");
  params.addParamNamesToGroup("partitioner centroid_partitioner_direction", "Partitioning");

  return params;
}


MooseMesh::MooseMesh(const InputParameters & parameters) :
    MooseObject(parameters),
    Restartable(parameters, "Mesh"),
    _mesh_distribution_type(getParam<MooseEnum>("distribution")),
    _mesh_parallel_type(getParam<MooseEnum>("parallel_type")),
    _use_distributed_mesh(false),
    _distribution_overridden(false),
    _parallel_type_overridden(false),
    _partitioner_name(getParam<MooseEnum>("partitioner")),
    _partitioner_overridden(false),
    _custom_partitioner_requested(false),
    _uniform_refine_level(0),
    _is_changed(false),
    _is_nemesis(getParam<bool>("nemesis")),
    _is_prepared(false),
    _needs_prepare_for_use(false),
    _node_to_elem_map_built(false),
    _node_to_active_semilocal_elem_map_built(false),
    _patch_size(40),
    _patch_update_strategy(getParam<MooseEnum>("patch_update_strategy")),
    _regular_orthogonal_mesh(false),
    _allow_recovery(true),
    _construct_node_list_from_side_list(getParam<bool>("construct_node_list_from_side_list"))
{
  // This flag is deprecated, but we still allow it to be used. It
  // will still do the same thing as it did before, but now it will
  // print a deprecated message.
  switch (_mesh_distribution_type)
  {
  case 0: // PARALLEL
    mooseDeprecated("Using 'distribution = PARALLEL' in the Mesh block is deprecated, use 'parallel_type = DISTRIBUTED' instead.");
    _use_distributed_mesh = true;
    break;

  case 1: // SERIAL
    mooseDeprecated("Using 'distribution = SERIAL' in the Mesh block is deprecated, use 'parallel_type = REPLICATED' instead.");
    if (_app.getDistributedMeshOnCommandLine() || _is_nemesis)
      _distribution_overridden = true;
    break;

  case 2: // DEFAULT
    // If the user did not specify any 'distribution = foo' in his
    // input file, there's nothing to do.  In particular, we do not
    // want to allow the command line to override the default mesh
    // type in this case.
    break;
  }

  switch (_mesh_parallel_type)
  {
  case 0: // PARALLEL
    _use_distributed_mesh = true;
    break;
  case 1: // SERIAL
    if (_app.getDistributedMeshOnCommandLine() || _is_nemesis)
      _parallel_type_overridden = true;
    break;
  case 2: // DEFAULT
    // The user did not specify 'distribution = XYZ' in the input file,
    // so we allow the --distributed-mesh command line arg to possibly turn
    // on DistributedMesh.  If the command line arg is not present, we pick ReplicatedMesh.
    if (_app.getDistributedMeshOnCommandLine())
      _use_distributed_mesh = true;

    break;
    // No default switch needed for MooseEnum
  }

  // If the user specifies 'nemesis = true' in the Mesh block, we
  // must use DistributedMesh.
  if (_is_nemesis)
    _use_distributed_mesh = true;

  unsigned dim = getParam<MooseEnum>("dim");

  if (_use_distributed_mesh)
  {
    _mesh = libmesh_make_unique<DistributedMesh>(_communicator, dim);
    if (_partitioner_name != "default" && _partitioner_name != "parmetis")
    {
      _partitioner_name = "parmetis";
      _partitioner_overridden = true;
    }

    // Add geometric ghosting functors to mesh if running with DistributedMesh
    if (getParam<bool>("ghost_point_neighbors"))
      _ghosting_functors.emplace_back(libmesh_make_unique<GhostPointNeighbors>(*_mesh));

    auto num_ghosted_layers = getParam<unsigned short>("num_ghosted_layers");
    if (num_ghosted_layers > 1)
    {
      auto default_coupling = libmesh_make_unique<DefaultCoupling>();
      default_coupling->set_n_levels(num_ghosted_layers);
      _ghosting_functors.emplace_back(std::move(default_coupling));
    }

    for (auto & gf : _ghosting_functors)
      _mesh->add_ghosting_functor(*gf);
  }
  else
    _mesh = libmesh_make_unique<ReplicatedMesh>(_communicator, dim);
}

MooseMesh::MooseMesh(const MooseMesh & other_mesh) :
    MooseObject(other_mesh._pars),
    Restartable(_pars, "Mesh"),
    _mesh_distribution_type(other_mesh._mesh_distribution_type),
    _mesh_parallel_type(other_mesh._mesh_parallel_type),
    _use_distributed_mesh(other_mesh._use_distributed_mesh),
    _distribution_overridden(other_mesh._distribution_overridden),
    _mesh(other_mesh.getMesh().clone()),
    _partitioner_name(other_mesh._partitioner_name),
    _partitioner_overridden(other_mesh._partitioner_overridden),
    _uniform_refine_level(other_mesh.uniformRefineLevel()),
    _is_changed(false),
    _is_nemesis(false),
    _is_prepared(false),
    _needs_prepare_for_use(false),
    _node_to_elem_map_built(false),
    _patch_size(40),
    _patch_update_strategy(other_mesh._patch_update_strategy),
    _regular_orthogonal_mesh(false),
    _construct_node_list_from_side_list(other_mesh._construct_node_list_from_side_list)
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
MooseMesh::prepare(bool force)
{
  if (dynamic_cast<DistributedMesh *>(&getMesh()) && !_is_nemesis)
  {
    // Call prepare_for_use() and allow renumbering
    getMesh().allow_renumbering(true);
    if (force || _needs_prepare_for_use)
      getMesh().prepare_for_use();
  }
  else
  {
    // Call prepare_for_use() and DO NOT allow renumbering
    getMesh().allow_renumbering(false);
    if (force || _needs_prepare_for_use)
      getMesh().prepare_for_use();
  }

  // Collect (local) subdomain IDs
  const MeshBase::element_iterator el_end = getMesh().elements_end();

  _mesh_subdomains.clear();
  for (MeshBase::element_iterator el = getMesh().elements_begin(); el != el_end; ++el)
    _mesh_subdomains.insert((*el)->subdomain_id());

  // Make sure nodesets have been generated
  buildNodeListFromSideList();

  // Collect (local) boundary IDs
  const std::set<BoundaryID> & local_bids = getMesh().get_boundary_info().get_boundary_ids();
  _mesh_boundary_ids.insert(local_bids.begin(), local_bids.end());

  const std::set<BoundaryID> & local_node_bids = getMesh().get_boundary_info().get_node_boundary_ids();
  _mesh_nodeset_ids.insert(local_node_bids.begin(), local_node_bids.end());

  const std::set<BoundaryID> & local_side_bids = getMesh().get_boundary_info().get_side_boundary_ids();
  _mesh_sideset_ids.insert(local_side_bids.begin(), local_side_bids.end());

  // Communicate subdomain and boundary IDs if this is a parallel mesh
  if (!getMesh().is_serial())
  {
    _communicator.set_union(_mesh_subdomains);
    _communicator.set_union(_mesh_boundary_ids);
    _communicator.set_union(_mesh_nodeset_ids);
    _communicator.set_union(_mesh_sideset_ids);
  }

  detectOrthogonalDimRanges();

  update();

  // Prepared has been called
  _is_prepared = true;
  _needs_prepare_for_use = false;
}

void
MooseMesh::update()
{
  // Rebuild the boundary conditions
  buildNodeListFromSideList();

  //Update the node to elem map
  _node_to_elem_map.clear();
  _node_to_elem_map_built = false;
  _node_to_active_semilocal_elem_map.clear();
  _node_to_active_semilocal_elem_map_built = false;

  buildNodeList();
  buildBndElemList();
  cacheInfo();
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

const Node*
MooseMesh::nodePtr(const dof_id_type i) const
{
  if (i > getMesh().max_node_id())
    return (*_quadrature_nodes.find(i)).second;

  return getMesh().node_ptr(i);
}

Node*
MooseMesh::nodePtr(const dof_id_type i)
{
  if (i > getMesh().max_node_id())
    return _quadrature_nodes[i];

  return getMesh().node_ptr(i);
}

void
MooseMesh::meshChanged()
{
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

  // Lets the output system know that the mesh has changed recently.
  _is_changed = true;

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
  ConstElemRange elem_range(getMesh().local_elements_begin(), getMesh().local_elements_end(), 1);
  CacheChangedListsThread cclt(*this);
  Threads::parallel_reduce(elem_range, cclt);

  _coarsened_element_children.clear();

  _refined_elements = libmesh_make_unique<ConstElemPointerRange>(cclt._refined_elements.begin(), cclt._refined_elements.end());
  _coarsened_elements = libmesh_make_unique<ConstElemPointerRange>(cclt._coarsened_elements.begin(), cclt._coarsened_elements.end());
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
  _semilocal_node_list.clear();

  // First add the nodes connected to local elems
  ConstElemRange * active_local_elems = getActiveLocalElementRange();
  for (const auto & elem : *active_local_elems)
  {
    for (unsigned int n = 0; n < elem->n_nodes(); ++n)
    {
      // Since elem is const here but we require a non-const Node * to
      // store in the _semilocal_node_list (otherwise things like
      // UpdateDisplacedMeshThread don't work), we are using the "old"
      // Elem interface to get a non-constant Node pointer from a
      // constant Elem.
      Node * node = elem->get_node(n);

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
  _active_semilocal_node_range = libmesh_make_unique<SemiLocalNodeRange>(_semilocal_node_list.begin(), _semilocal_node_list.end());
}

bool
MooseMesh::isSemiLocal(Node * node)
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
  BndNodeCompare(){}

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
  freeBndNodes();

  /// Boundary node list (node ids and corresponding side-set ids, arrays always have the same length)
  std::vector<dof_id_type> nodes;
  std::vector<boundary_id_type> ids;
  getMesh().get_boundary_info().build_node_list(nodes, ids);

  int n = nodes.size();
  _bnd_nodes.resize(n);
  for (int i = 0; i < n; i++)
  {
    _bnd_nodes[i] = new BndNode(&getMesh().node(nodes[i]), ids[i]);
    _node_set_nodes[ids[i]].push_back(nodes[i]);
    _bnd_node_ids[ids[i]].insert(nodes[i]);
  }

  _bnd_nodes.reserve(_bnd_nodes.size() + _extra_bnd_nodes.size());
  for (unsigned int i = 0; i < _extra_bnd_nodes.size(); i++)
  {
    BndNode * bnode = new BndNode(_extra_bnd_nodes[i]._node, _extra_bnd_nodes[i]._bnd_id);
    _bnd_nodes.push_back(bnode);
    _bnd_node_ids[ids[i]].insert(_extra_bnd_nodes[i]._node->id());
  }

  BndNodeCompare mein_kompfare;

  // This sort is here so that boundary conditions are always applied in the same order
  std::sort(_bnd_nodes.begin(), _bnd_nodes.end(), mein_kompfare);
}

void
MooseMesh::buildBndElemList()
{
  freeBndElems();

  /// Boundary node list (node ids and corresponding side-set ids, arrays always have the same length)
  std::vector<dof_id_type> elems;
  std::vector<unsigned short int> sides;
  std::vector<boundary_id_type> ids;
  getMesh().get_boundary_info().build_active_side_list(elems, sides, ids);

  int n = elems.size();
  _bnd_elems.resize(n);
  for (int i = 0; i < n; i++)
  {
    _bnd_elems[i] = new BndElement(getMesh().elem_ptr(elems[i]), sides[i], ids[i]);
    _bnd_elem_ids[ids[i]].insert(elems[i]);
  }
}

const std::map<dof_id_type, std::vector<dof_id_type> > &
MooseMesh::nodeToElemMap()
{
  if (!_node_to_elem_map_built) // Guard the creation with a double checked lock
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    if (!_node_to_elem_map_built)
    {
      MeshBase::const_element_iterator       el  = getMesh().elements_begin();
      const MeshBase::const_element_iterator end = getMesh().elements_end();

      for (; el != end; ++el)
        for (unsigned int n = 0; n < (*el)->n_nodes(); n++)
          _node_to_elem_map[(*el)->node(n)].push_back((*el)->id());

      _node_to_elem_map_built = true; // MUST be set at the end for double-checked locking to work!
    }
  }

  return _node_to_elem_map;
}

const std::map<dof_id_type, std::vector<dof_id_type> > &
MooseMesh::nodeToActiveSemilocalElemMap()
{
  if (!_node_to_active_semilocal_elem_map_built) // Guard the creation with a double checked lock
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    if (!_node_to_active_semilocal_elem_map_built)
    {
      MeshBase::const_element_iterator       el  = getMesh().semilocal_elements_begin();
      const MeshBase::const_element_iterator end = getMesh().semilocal_elements_end();

      for (; el != end; ++el)
        if ((*el)->active())
          for (unsigned int n = 0; n < (*el)->n_nodes(); n++)
            _node_to_active_semilocal_elem_map[(*el)->node(n)].push_back((*el)->id());

      _node_to_active_semilocal_elem_map_built = true; // MUST be set at the end for double-checked locking to work!
    }
  }

  return _node_to_active_semilocal_elem_map;
}


ConstElemRange *
MooseMesh::getActiveLocalElementRange()
{
  if (!_active_local_elem_range)
    _active_local_elem_range = libmesh_make_unique<ConstElemRange>(getMesh().active_local_elements_begin(),
                                                                   getMesh().active_local_elements_end(), GRAIN_SIZE);

  return _active_local_elem_range.get();
}

NodeRange *
MooseMesh::getActiveNodeRange()
{
  if (!_active_node_range)
    _active_node_range = libmesh_make_unique<NodeRange>(getMesh().active_nodes_begin(),
                                                        getMesh().active_nodes_end(), GRAIN_SIZE);

  return _active_node_range.get();
}

SemiLocalNodeRange *
MooseMesh::getActiveSemiLocalNodeRange() const
{
  mooseAssert(_active_semilocal_node_range, "_active_semilocal_node_range has not been created yet!");

  return _active_semilocal_node_range.get();
}

ConstNodeRange *
MooseMesh::getLocalNodeRange()
{
  if (!_local_node_range)
    _local_node_range = libmesh_make_unique<ConstNodeRange>(getMesh().local_nodes_begin(),
                                                            getMesh().local_nodes_end(), GRAIN_SIZE);

  return _local_node_range.get();
}

ConstBndNodeRange *
MooseMesh::getBoundaryNodeRange()
{
  if (!_bnd_node_range)
    _bnd_node_range = libmesh_make_unique<ConstBndNodeRange>(bndNodesBegin(),
                                                             bndNodesEnd(), GRAIN_SIZE);

  return _bnd_node_range.get();
}

ConstBndElemRange *
MooseMesh::getBoundaryElementRange()
{
  if (!_bnd_elem_range)
    _bnd_elem_range = libmesh_make_unique<ConstBndElemRange>(bndElemsBegin(),
                                                             bndElemsEnd(), GRAIN_SIZE);

  return _bnd_elem_range.get();
}

void
MooseMesh::cacheInfo()
{
  const MeshBase::element_iterator end = getMesh().elements_end();

  // TODO: Thread this!
  for (MeshBase::element_iterator el = getMesh().elements_begin(); el != end; ++el)
  {
    Elem * elem = *el;

    SubdomainID subdomain_id = elem->subdomain_id();

    for (unsigned int side = 0; side < elem->n_sides(); side++)
    {
      std::vector<BoundaryID> boundaryids = getBoundaryIDs(elem, side);

      std::set<BoundaryID> & subdomain_set = _subdomain_boundary_ids[subdomain_id];

      subdomain_set.insert(boundaryids.begin(), boundaryids.end());
    }

    for (unsigned int nd = 0; nd < elem->n_nodes(); ++nd)
    {
      Node & node = *elem->node_ptr(nd);
      _block_node_list[node.id()].insert(elem->subdomain_id());
    }
  }
}

const std::set<SubdomainID> &
MooseMesh::getNodeBlockIds(const Node & node) const
{
  std::map<dof_id_type, std::set<SubdomainID> >::const_iterator it = _block_node_list.find(node.id());

  if (it == _block_node_list.end())
    mooseError("Unable to find node: " << node.id() << " in any block list.");

  return it->second;
}

// default begin() accessor
MooseMesh::bnd_node_iterator
MooseMesh::bndNodesBegin ()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.begin(), _bnd_nodes.end(), p);
}

// default end() accessor
MooseMesh::bnd_node_iterator
MooseMesh::bndNodesEnd ()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.end(), _bnd_nodes.end(), p);
}

// default begin() accessor
MooseMesh::bnd_elem_iterator
MooseMesh::bndElemsBegin ()
{
  Predicates::NotNull<bnd_elem_iterator_imp> p;
  return bnd_elem_iterator(_bnd_elems.begin(), _bnd_elems.end(), p);
}

// default end() accessor
MooseMesh::bnd_elem_iterator
MooseMesh::bndElemsEnd ()
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
    const libMesh::MeshBase::node_iterator end = getMesh().nodes_end();
    for (libMesh::MeshBase::node_iterator i = getMesh().nodes_begin(); i != end; ++i)
    {
      _node_map.push_back(*i);
    }
  }

  Node *node = nullptr;
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
MooseMesh::addQuadratureNode(const Elem * elem, const unsigned short int side, const unsigned int qp, BoundaryID bid, const Point & point)
{
  Node * qnode;

  if (_elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].find(qp) == _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].end())
  {
    // Create a new node id starting from the max node id and counting down.  This will be the least
    // likely to collide with an existing node id.
    // Note that we are using numeric_limits<unsigned>::max even
    // though max_id is stored as a dof_id_type.  I tried this with
    // numeric_limits<dof_id_type>::max and it broke several tests in
    // MOOSE.  So, this is some kind of a magic number that we will
    // just continue to use...
    dof_id_type max_id = std::numeric_limits<unsigned int>::max()-100;
    dof_id_type new_id = max_id - _quadrature_nodes.size();

    if (new_id <= getMesh().max_node_id())
      mooseError("Quadrature node id collides with existing node id!");

    qnode = new Node(point, new_id);

    // Keep track of this new node in two different ways for easy lookup
    _quadrature_nodes[new_id] = qnode;
    _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side][qp] = qnode;

    _node_to_elem_map[new_id].push_back(elem->id());
    if (elem->active())
      _node_to_active_semilocal_elem_map[new_id].push_back(elem->id());
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
MooseMesh::getQuadratureNode(const Elem * elem, const unsigned short int side, const unsigned int qp)
{
  mooseAssert(_elem_to_side_to_qp_to_quadrature_nodes.find(elem->id()) != _elem_to_side_to_qp_to_quadrature_nodes.end(), "Elem has no quadrature nodes!");
  mooseAssert(_elem_to_side_to_qp_to_quadrature_nodes[elem->id()].find(side) != _elem_to_side_to_qp_to_quadrature_nodes[elem->id()].end(), "Side has no quadrature nodes!");
  mooseAssert(_elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].find(qp) != _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].end(), "qp not found on side!");

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

  BoundaryID id = Moose::INVALID_BOUNDARY_ID;
  std::istringstream ss(boundary_name);

  if (!(ss >> id))
    id = getMesh().get_boundary_info().get_id_by_name(boundary_name);

  return id;
}

std::vector<BoundaryID>
MooseMesh::getBoundaryIDs(const std::vector<BoundaryName> & boundary_name, bool generate_unknown) const
{
  const BoundaryInfo & boundary_info = getMesh().get_boundary_info();
  const std::map<BoundaryID, std::string> & sideset_map = boundary_info.get_sideset_name_map();
  const std::map<BoundaryID, std::string> & nodeset_map = boundary_info.get_nodeset_name_map();

  std::set<BoundaryID> boundary_ids = boundary_info.get_boundary_ids();
  BoundaryID max_boundary_id = boundary_ids.empty() ? 0 : *(boundary_ids.rbegin());

  std::vector<BoundaryID> ids(boundary_name.size());
  for (unsigned int i = 0; i < boundary_name.size(); i++)
  {
    if (boundary_name[i] == "ANY_BOUNDARY_ID")
    {
      ids.assign(_mesh_boundary_ids.begin(), _mesh_boundary_ids.end());
      if (i)
        mooseWarning("You passed \"ANY_BOUNDARY_ID\" in addition to other boundary_names.  This may be a logic error.");
      break;
    }

    BoundaryID id;
    std::istringstream ss(boundary_name[i]);

    if (!(ss >> id))
    {
      /**
       * If the conversion from a name to a number fails, that means that this must be a named
       * boundary.  We will look in the complete map for this sideset and create a new name/ID pair
       * if requested.
       */
      if (generate_unknown
          && !MooseUtils::doesMapContainValue(sideset_map, std::string(boundary_name[i]))
          && !MooseUtils::doesMapContainValue(nodeset_map, std::string(boundary_name[i])))
        id = ++max_boundary_id;
      else
        id = boundary_info.get_id_by_name(boundary_name[i]);
    }

    ids[i] = id;
  }

  return ids;
}

SubdomainID
MooseMesh::getSubdomainID(const SubdomainName & subdomain_name) const
{
  if (subdomain_name == "ANY_BLOCK_ID")
    mooseError("Please use getSubdomainIDs() when passing \"ANY_BLOCK_ID\"");

  SubdomainID id = Moose::INVALID_BLOCK_ID;
  std::istringstream ss(subdomain_name);

  if (!(ss >> id))
    id = getMesh().get_id_by_name(subdomain_name);

  return id;
}

std::vector<SubdomainID>
MooseMesh::getSubdomainIDs(const std::vector<SubdomainName> & subdomain_name) const
{
  std::vector<SubdomainID> ids(subdomain_name.size());

  for (unsigned int i = 0; i < subdomain_name.size(); i++)
  {
    if (subdomain_name[i] == "ANY_BLOCK_ID")
    {
      ids.assign(_mesh_subdomains.begin(), _mesh_subdomains.end());
      if (i)
        mooseWarning("You passed \"ANY_BLOCK_ID\" in addition to other block names.  This may be a logic error.");
      break;
    }

    SubdomainID id = Moose::INVALID_BLOCK_ID;
    std::istringstream ss(subdomain_name[i]);

    if (!(ss >> id))
      id = getMesh().get_id_by_name(subdomain_name[i]);

    ids[i] = id;
  }

  return ids;
}

void
MooseMesh::setSubdomainName(SubdomainID subdomain_id, SubdomainName name)
{
  getMesh().subdomain_name(subdomain_id) = name;
}

void
MooseMesh::setBoundaryName(BoundaryID boundary_id, BoundaryName name)
{
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  std::vector<BoundaryID> side_boundaries;
  boundary_info.build_side_boundary_ids(side_boundaries);

  // We need to figure out if this boundary is a sideset or nodeset
  if (std::find(side_boundaries.begin(), side_boundaries.end(), boundary_id) != side_boundaries.end())
    boundary_info.sideset_name(boundary_id) = name;
  else
    boundary_info.nodeset_name(boundary_id) = name;
}

void
MooseMesh::buildPeriodicNodeMap(std::multimap<dof_id_type, dof_id_type> & periodic_node_map, unsigned int var_number, PeriodicBoundaries *pbs) const
{
  mooseAssert(!Threads::in_threads, "This function should only be called outside of a threaded region due to the use of PointLocator");

  periodic_node_map.clear();

  MeshBase::const_element_iterator it = getMesh().active_elements_begin();
  MeshBase::const_element_iterator it_end = getMesh().active_elements_end();
  std::unique_ptr<PointLocatorBase> point_locator = getMesh().sub_point_locator();

  // Get a const reference to the BoundaryInfo object that we will use several times below...
  const BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  // A typedef makes the code below easier to read...
  typedef std::multimap<dof_id_type, dof_id_type>::iterator IterType;

  // Container to catch IDs passed back from the BoundaryInfo object
  std::vector<boundary_id_type> bc_ids;

  for (; it != it_end; ++it)
  {
    const Elem *elem = *it;
    for (unsigned int s = 0; s < elem->n_sides(); ++s)
    {
      if (elem->neighbor(s))
        continue;

      boundary_info.boundary_ids (elem, s, bc_ids);
      for (const auto & boundary_id : bc_ids)
      {
        const PeriodicBoundaryBase * periodic = pbs->boundary(boundary_id);
        if (periodic && periodic->is_my_variable(var_number))
        {
          const Elem * neigh = pbs->neighbor(boundary_id, *point_locator, elem, s);
          unsigned int s_neigh = boundary_info.side_with_boundary_id (neigh, periodic->pairedboundary);

          std::unique_ptr<Elem> elem_side = elem->build_side(s);
          std::unique_ptr<Elem> neigh_side = neigh->build_side(s_neigh);

          // At this point we have matching sides - lets find matching nodes
          for (unsigned int i = 0; i < elem_side->n_nodes(); ++i)
          {
            const Node * master_node = elem->node_ptr(i);
            Point master_point = periodic->get_corresponding_pos(*master_node);
            for (unsigned int j = 0; j < neigh_side->n_nodes(); ++j)
            {
              Node *slave_node = neigh_side->node_ptr(j);
              if (master_point.absolute_fuzzy_equals(*slave_node))
              {
                // Avoid inserting any duplicates
                std::pair<IterType, IterType> iters =
                  periodic_node_map.equal_range(master_node->id());
                bool found = false;
                for (IterType map_it = iters.first; map_it != iters.second; ++map_it)
                  if (map_it->second == slave_node->id())
                    found = true;
                if (!found)
                {
                  periodic_node_map.insert(std::make_pair(master_node->id(), slave_node->id()));
                  periodic_node_map.insert(std::make_pair(slave_node->id(), master_node->id()));
                }
              }
            }
          }
        }
      }
    }
  }
}

void
MooseMesh::buildPeriodicNodeSets(std::map<BoundaryID, std::set<dof_id_type> > & periodic_node_sets,
                                 unsigned int var_number,
                                 PeriodicBoundaries *pbs) const
{
  periodic_node_sets.clear();

  std::vector<dof_id_type> nl;
  std::vector<boundary_id_type> il;

  getMesh().get_boundary_info().build_node_list(nl, il);

  // Loop over all the boundary nodes adding the periodic nodes to the appropriate set
  for (unsigned int i = 0; i < nl.size(); ++i)
  {
    // Is this current node on a known periodic boundary?
    if (periodic_node_sets.find(il[i]) != periodic_node_sets.end())
      periodic_node_sets[il[i]].insert(nl[i]);
    else // This still might be a periodic node but we just haven't seen this boundary_id yet
    {
      const PeriodicBoundaryBase * periodic = pbs->boundary(il[i]);
      if (periodic && periodic->is_my_variable(var_number))
        periodic_node_sets[il[i]].insert(nl[i]);
    }
  }
}

bool
MooseMesh::detectOrthogonalDimRanges(Real tol)
{
  if (_regular_orthogonal_mesh)
    return true;

  std::vector<Real> min(3, std::numeric_limits<Real>::max());
  std::vector<Real> max(3, std::numeric_limits<Real>::min());
  unsigned int dim = getMesh().mesh_dimension();

  // Find the bounding box of our mesh
  const MeshBase::node_iterator nd_end = getMesh().nodes_end();
  for (MeshBase::node_iterator nd = getMesh().nodes_begin(); nd != nd_end; ++nd)
  {
    Node & node = **nd;
    for (unsigned int i = 0; i < dim; ++i)
    {
      if (node(i) < min[i])
        min[i] = node(i);
      if (node(i) > max[i])
        max[i] = node(i);
    }
  }

  _extreme_nodes.resize(8);  // 2^LIBMESH_DIM
  // Now make sure that there are actual nodes at all of the extremes
  unsigned int extreme_matches = 0;
  std::vector<unsigned int> comp_map(3);
  for (MeshBase::node_iterator nd = getMesh().nodes_begin(); nd != nd_end; ++nd)
  {
    // See if the current node is located at one of the extremes
    Node & node = **nd;
    unsigned int coord_match = 0;

    for (unsigned int i = 0; i < dim; ++i)
    {
      if (std::abs(node(i) - min[i]) < tol)
      {
        comp_map[i] = MIN;
        ++coord_match;
      }
      else if (std::abs(node(i) - max[i]) < tol)
      {
        comp_map[i] = MAX;
        ++coord_match;
      }
    }

    if (coord_match == dim)  // Found a coordinate at one of the extremes
    {
      _extreme_nodes[comp_map[X] * 4 + comp_map[Y] * 2 + comp_map[Z]] = &node;
      ++extreme_matches;
    }
  }

  // See if we matched all of the extremes for the mesh dimension
  if (extreme_matches != std::pow(2.0, (int)dim))
    return false;                    // This is not a regular orthogonal mesh

  // This is a regular orthogonal mesh, so set the bounds
  _regular_orthogonal_mesh = true;
  _bounds.resize(LIBMESH_DIM);
  for (unsigned int i = 0; i < dim; ++i)
  {
    _bounds[i].resize(2);
    _bounds[i][MIN] = min[i];
    _bounds[i][MAX] = max[i];
  }
  for (unsigned int i = dim; i < LIBMESH_DIM; ++i)
  {
    _bounds[i].resize(2);
    _bounds[i][MIN] = 0;
    _bounds[i][MAX] = 0;
  }

  return true;
}

void
MooseMesh::detectPairedSidesets()
{
  // Loop over level-0 elements (since boundary condition information
  // is only directly stored for them) and find sidesets with normals
  // that point in the -x, +x, -y, +y, and -z, +z direction.  If there
  // is a unique sideset id for each direction, then the paired
  // sidesets consist of (-x,+x), (-y,+y), (-z,+z).  If there are
  // multiple sideset ids for a given direction, then we can't pick a
  // single pair for that direction.  In that case, we'll just return
  // as was done in the original algorithm.

  // Points used for direction comparison
  const Point
    minus_x(-1,  0,  0),
    plus_x ( 1,  0,  0),
    minus_y( 0, -1,  0),
    plus_y ( 0,  1,  0),
    minus_z( 0,  0, -1),
    plus_z ( 0,  0,  1);

  // we need to test all element dimensions from dim down to 1
  const unsigned int dim = getMesh().mesh_dimension();

  // boundary id sets for elements of different dimensions
  std::vector<std::set<BoundaryID>>
    minus_x_ids(dim), plus_x_ids(dim),
    minus_y_ids(dim), plus_y_ids(dim),
    minus_z_ids(dim), plus_z_ids(dim);

  std::vector<std::unique_ptr<FEBase>> fe_faces(dim);
  std::vector<std::unique_ptr<QGauss>> qfaces(dim);
  for (unsigned side_dim = 0; side_dim < dim; ++side_dim)
  {
    // Face is assumed to be flat, therefore normal is assumed to be
    // constant over the face, therefore only compute it at 1 qp.
    qfaces[side_dim] = std::unique_ptr<QGauss>(new QGauss(side_dim, CONSTANT));

    // A first-order Lagrange FE for the face.
    fe_faces[side_dim] = FEBase::build(side_dim + 1,
                                       FEType(FIRST, LAGRANGE));
    fe_faces[side_dim]->attach_quadrature_rule(qfaces[side_dim].get());
  }

  // We need this to get boundary ids for each boundary face we encounter.
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();
  std::vector<boundary_id_type> face_ids;

  MeshBase::const_element_iterator       el     = getMesh().level_elements_begin(0);
  const MeshBase::const_element_iterator end_el = getMesh().level_elements_end(0);
  for (; el != end_el ; ++el)
  {
    Elem * elem = *el;

    // dimension of the current element and its normals
    unsigned int side_dim = elem->dim() - 1;
    const std::vector<Point> & normals = fe_faces[side_dim]->get_normals();

    // loop over element sides
    for (unsigned int s = 0; s < elem->n_sides(); s++)
    {
      // If side is on the boundary
      if (elem->neighbor(s) == nullptr)
      {
        std::unique_ptr<Elem> side = elem->build_side(s);

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

  for (unsigned side_dim = 0; side_dim < dim; ++side_dim)
  {
    // If unique pairings were found, fill up the _paired_boundary data
    // structure with that information.
    if (minus_x_ids[side_dim].size() == 1 && plus_x_ids[side_dim].size() == 1)
      _paired_boundary.emplace_back(std::make_pair(*(minus_x_ids[side_dim].begin()),
                                                   *(plus_x_ids[side_dim].begin())));

    if (minus_y_ids[side_dim].size() == 1 && plus_y_ids[side_dim].size() == 1)
      _paired_boundary.emplace_back(std::make_pair(*(minus_y_ids[side_dim].begin()),
                                                   *(plus_y_ids[side_dim].begin())));

    if (minus_z_ids[side_dim].size() == 1 && plus_z_ids[side_dim].size() == 1)
      _paired_boundary.emplace_back(std::make_pair(*(minus_z_ids[side_dim].begin()),
                                                   *(plus_z_ids[side_dim].begin())));
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
  mooseAssert(_regular_orthogonal_mesh, "The current mesh is not a regular orthogonal mesh");
  mooseAssert(component < LIBMESH_DIM, "Requested dimension out of bounds");

  return _bounds[component][MIN];
}

Real
MooseMesh::getMaxInDimension(unsigned int component) const
{
  mooseAssert(_regular_orthogonal_mesh, "The current mesh is not a regular orthogonal mesh");
  mooseAssert(component < LIBMESH_DIM, "Requested dimension out of bounds");

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
  mooseAssert(_regular_orthogonal_mesh, "The current mesh is not a regular orthogonal mesh");
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

  return q-p;
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
    mooseError("Trying to retrieve automatic paired mapping for a mesh that is not regular and orthogonal");

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
  MeshBase::const_element_iterator       el     = getMesh().elements_begin();
  const MeshBase::const_element_iterator end_el = getMesh().elements_end();

  std::map<ElemType, Elem *> canonical_elems;

  // First, loop over all elements and find a canonical element for each type
  // Doing it this way guarantees that this is going to work in parallel
  for (; el != end_el ; ++el) // TODO: Thread this
  {
    Elem * elem = *el;
    ElemType type = elem->type();

    if (canonical_elems.find(type) == canonical_elems.end()) // If we haven't seen this type of elem before save it
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
    assembly->reinit(elem);
    assembly->reinit(elem, 0);
    QBase * qrule = assembly->qRule();
    QBase * qrule_face = assembly->qRuleFace();

    // Volume to volume projection for refinement
    buildRefinementMap(*elem, *qrule, *qrule_face, -1,-1,-1);

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
      for (unsigned int side = 0; side < elem->n_sides(); ++side) // Assume children have the same number of sides!
        if (!elem->is_child_on_side(child, side)) // Otherwise we already computed that map
          buildRefinementMap(*elem, *qrule, *qrule_face, -1, child, side);
  }
}

void
MooseMesh::buildRefinementMap(const Elem & elem, QBase & qrule, QBase & qrule_face, int parent_side, int child, int child_side)
{
  if (child == -1) // Doing volume mapping or parent side mapping
  {
    mooseAssert(parent_side == child_side, "Parent side must match child_side if not passing a specific child!");

    std::pair<int, ElemType> the_pair(parent_side, elem.type());

    if (_elem_type_to_refinement_map.find(the_pair) != _elem_type_to_refinement_map.end())
      mooseError("Already built a qp refinement map!");

    std::vector<std::pair<unsigned int, QpMap> > coarsen_map;
    std::vector<std::vector<QpMap> > & refinement_map = _elem_type_to_refinement_map[the_pair];
    findAdaptivityQpMaps(&elem, qrule, qrule_face, refinement_map, coarsen_map, parent_side, child, child_side);
  }
  else // Need to map a child side to parent volume qps
  {
    std::pair<int, int> child_pair(child, child_side);

    if (_elem_type_to_child_side_refinement_map.find(elem.type()) != _elem_type_to_child_side_refinement_map.end() &&
       _elem_type_to_child_side_refinement_map[elem.type()].find(child_pair) != _elem_type_to_child_side_refinement_map[elem.type()].end())
      mooseError("Already built a qp refinement map!");

    std::vector<std::pair<unsigned int, QpMap> > coarsen_map;
    std::vector<std::vector<QpMap> > & refinement_map = _elem_type_to_child_side_refinement_map[elem.type()][child_pair];
    findAdaptivityQpMaps(&elem, qrule, qrule_face, refinement_map, coarsen_map, parent_side, child, child_side);
  }

}

const std::vector<std::vector<QpMap> > &
MooseMesh::getRefinementMap(const Elem & elem, int parent_side, int child, int child_side)
{
  if (child == -1) // Doing volume mapping or parent side mapping
  {
    mooseAssert(parent_side == child_side, "Parent side must match child_side if not passing a specific child!");

    std::pair<int, ElemType> the_pair(parent_side, elem.type());

    if (_elem_type_to_refinement_map.find(the_pair) == _elem_type_to_refinement_map.end())
      mooseError("Could not find a suitable qp refinement map!");

    return _elem_type_to_refinement_map[the_pair];
  }
  else // Need to map a child side to parent volume qps
  {
    std::pair<int, int> child_pair(child, child_side);

    if (_elem_type_to_child_side_refinement_map.find(elem.type()) == _elem_type_to_child_side_refinement_map.end() ||
       _elem_type_to_child_side_refinement_map[elem.type()].find(child_pair) == _elem_type_to_child_side_refinement_map[elem.type()].end())
      mooseError("Could not find a suitable qp refinement map!");

    return _elem_type_to_child_side_refinement_map[elem.type()][child_pair];
  }

  /**
   *  TODO: When running with parallel mesh + stateful adaptivty we will need to make sure that each
   *  processor has a complete map.  This may require parallel communication.  This is likely to happen
   *  when running on a mixed element mesh.
   */
}

void
MooseMesh::buildCoarseningMap(const Elem & elem, QBase & qrule, QBase & qrule_face, int input_side)
{
  std::pair<int, ElemType> the_pair(input_side, elem.type());

  if (_elem_type_to_coarsening_map.find(the_pair) != _elem_type_to_coarsening_map.end())
    mooseError("Already built a qp coarsening map!");

  std::vector<std::vector<QpMap> > refinement_map;
  std::vector<std::pair<unsigned int, QpMap> > & coarsen_map = _elem_type_to_coarsening_map[the_pair];

  // The -1 here is for a specific child.  We don't do that for coarsening maps
  // Also note that we're always mapping the same side to the same side (which is guaranteed by libMesh).
  findAdaptivityQpMaps(&elem, qrule, qrule_face, refinement_map, coarsen_map, input_side, -1, input_side);

  /**
   *  TODO: When running with parallel mesh + stateful adaptivty we will need to make sure that each
   *  processor has a complete map.  This may require parallel communication.  This is likely to happen
   *  when running on a mixed element mesh.
   */
}

const std::vector<std::pair<unsigned int, QpMap> > &
MooseMesh::getCoarseningMap(const Elem & elem, int input_side)
{
  std::pair<int, ElemType> the_pair(input_side, elem.type());

  if (_elem_type_to_coarsening_map.find(the_pair) == _elem_type_to_coarsening_map.end())
    mooseError("Could not find a suitable qp refinement map!");

  return _elem_type_to_coarsening_map[the_pair];
}

void
MooseMesh::mapPoints(const std::vector<Point> & from, const std::vector<Point> & to, std::vector<QpMap> & qp_map)
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
                                std::vector<std::vector<QpMap> > & refinement_map,
                                std::vector<std::pair<unsigned int, QpMap> > & coarsen_map,
                                int parent_side,
                                int child,
                                int child_side)
{
  ReplicatedMesh mesh(_communicator);
  mesh.skip_partitioning(true);

  unsigned int dim = template_elem->dim();
  mesh.set_mesh_dimension(dim);

  for (unsigned int i = 0; i < template_elem->n_nodes(); ++i)
    mesh.add_point(template_elem->point(i));

  Elem * elem = mesh.add_elem(Elem::build(template_elem->type()).release());

  for (unsigned int i = 0; i < template_elem->n_nodes(); ++i)
    elem->set_node(i) = mesh.node_ptr(i);

  std::unique_ptr<FEBase> fe (FEBase::build(dim, FEType()));
  fe->get_phi();
  const std::vector<Point> & q_points_volume = fe->get_xyz();

  std::unique_ptr<FEBase> fe_face (FEBase::build(dim, FEType()));
  fe_face->get_phi();
  const std::vector<Point> & q_points_face = fe_face->get_xyz();

  fe->attach_quadrature_rule (&qrule);
  fe_face->attach_quadrature_rule (&qrule_face);

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

  std::map<unsigned int, std::vector<Point> > child_to_ref_points;

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

    const Elem * child_elem = elem->child(child);

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
MooseMesh::changeBoundaryId(const boundary_id_type old_id, const boundary_id_type new_id, bool delete_prev)
{
  // Get a reference to our BoundaryInfo object, we will use it several times below...
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  // Container to catch ids passed back from BoundaryInfo
  std::vector<boundary_id_type> old_ids;

  // Only level-0 elements store BCs.  Loop over them.
  MeshBase::element_iterator           el = getMesh().level_elements_begin(0);
  const MeshBase::element_iterator end_el = getMesh().level_elements_end(0);
  for (; el != end_el; ++el)
  {
    Elem * elem = *el;
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

void
MooseMesh::init()
{
  if (_custom_partitioner_requested)
  {
    // Check of partitioner is supplied (not allowed if custom partitioner is used)
    if (!parameters().isParamSetByAddParam("partitioner"))
      mooseError("If partitioner block is provided, partitioner keyword cannot be used!");
    // Set custom partitioner
    if (!_custom_partitioner.get())
      mooseError("Custom partitioner requested but not set!");
    getMesh().partitioner().reset(_custom_partitioner.release());
  }
  else
  {
    // Set standard partitioner
    // Set the partitioner based on partitioner name
    switch (_partitioner_name)
    {
    case -3: // default
      // We'll use the default partitioner, but notify the user of which one is being used...
      if (_use_distributed_mesh)
        _partitioner_name = "parmetis";
      else
        _partitioner_name = "metis";
      break;

    // No need to explicitily create the metis or parmetis partitioners,
    // They are the default for serial and parallel mesh respectively
    case -2: // metis
    case -1: // parmetis
      break;

    case 0: // linear
      getMesh().partitioner().reset(new LinearPartitioner);
      break;
    case 1: // centroid
    {
      if (!isParamValid("centroid_partitioner_direction"))
        mooseError("If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

      MooseEnum direction = getParam<MooseEnum>("centroid_partitioner_direction");

      if (direction == "x")
        getMesh().partitioner().reset(new CentroidPartitioner(CentroidPartitioner::X));
      else if (direction == "y")
        getMesh().partitioner().reset(new CentroidPartitioner(CentroidPartitioner::Y));
      else if (direction == "z")
        getMesh().partitioner().reset(new CentroidPartitioner(CentroidPartitioner::Z));
      else if (direction == "radial")
        getMesh().partitioner().reset(new CentroidPartitioner(CentroidPartitioner::RADIAL));
      break;
    }
    case 2: // hilbert_sfc
      getMesh().partitioner().reset(new HilbertSFCPartitioner);
      break;
    case 3: // morton_sfc
      getMesh().partitioner().reset(new MortonSFCPartitioner);
      break;
    }
  }

  if (_app.isRecovering() && _allow_recovery && _app.isUltimateMaster())
    // For now, only read the recovery mesh on the Ultimate Master.. sub-apps need to just build their mesh like normal
    getMesh().read(_app.getRecoverFileBase() + "_mesh.cpr");
  else // Normally just build the mesh
    buildMesh();
}

unsigned int
MooseMesh::dimension() const
{
  return getMesh().mesh_dimension();
}

std::vector<BoundaryID>
MooseMesh::getBoundaryIDs(const Elem *const elem, const unsigned short int side) const
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

template <>
const std::set<SubdomainID> &
MooseMesh::getBlockOrBoundaryIDs() const
{
  return meshSubdomains();
}

template <>
const std::set<BoundaryID> &
MooseMesh::getBlockOrBoundaryIDs() const
{
  return getBoundaryIDs();
}

template <>
SubdomainID
MooseMesh::getAnyID() const
{
  return Moose::ANY_BLOCK_ID;
}

template <>
BoundaryID
MooseMesh::getAnyID() const
{
  return Moose::ANY_BOUNDARY_ID;
}

void
MooseMesh::buildNodeListFromSideList()
{
  if (_construct_node_list_from_side_list)
    getMesh().get_boundary_info().build_node_list_from_side_list();
}

void
MooseMesh::buildSideList(std::vector<dof_id_type> & el, std::vector<unsigned short int> & sl, std::vector<boundary_id_type> & il)
{
  getMesh().get_boundary_info().build_side_list(el, sl, il);
}

unsigned int
MooseMesh::sideWithBoundaryID(const Elem * const elem, const BoundaryID boundary_id) const
{
  return getMesh().get_boundary_info().side_with_boundary_id(elem, boundary_id);
}

MeshBase::const_node_iterator
MooseMesh::localNodesBegin()
{
  return getMesh().local_nodes_begin();
}

MeshBase::const_node_iterator
MooseMesh::localNodesEnd()
{
  return getMesh().local_nodes_end();
}

MeshBase::const_element_iterator
MooseMesh::activeLocalElementsBegin()
{
  return getMesh().active_local_elements_begin();
}

const MeshBase::const_element_iterator
MooseMesh::activeLocalElementsEnd()
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

bool
MooseMesh::changed() const
{
  return _is_changed;
}

void
MooseMesh::changed(bool state)
{
  _is_changed = state;
}

bool
MooseMesh::prepared() const
{
  return _is_prepared;
}

void
MooseMesh::prepared(bool state)
{
  _is_prepared = state;
}

void
MooseMesh::needsPrepareForUse()
{
  _needs_prepare_for_use = true;
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
MooseMesh::setBoundaryToNormalMap(std::unique_ptr<std::map<BoundaryID, RealVectorValue>> boundary_map)
{
  _boundary_to_normal_map = std::move(boundary_map);
}

void
MooseMesh::setBoundaryToNormalMap(std::map<BoundaryID, RealVectorValue> * boundary_map)
{
  mooseDeprecated("setBoundaryToNormalMap(std::map<BoundaryID, RealVectorValue> * boundary_map) is deprecated, use the unique_ptr version instead");
  _boundary_to_normal_map.reset(boundary_map);
}

unsigned int
MooseMesh::uniformRefineLevel() const
{
  return _uniform_refine_level;
}

void
MooseMesh::setUniformRefineLevel(unsigned int level)
{
  _uniform_refine_level = level;
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

namespace // Anonymous namespace for helper
{
  // A class for templated methods that expect output iterator
  // arguments, which adds objects to the Mesh.
  // Although any mesh_inserter_iterator can add any object, we
  // template it around object type so that type inference and
  // iterator_traits will work.
  // This object specifically is used to insert extra ghost elems into the mesh
  template <typename T>
  struct extra_ghost_elem_inserter : std::iterator<std::output_iterator_tag, T>
  {
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

} // anonymous namespace


void
MooseMesh::ghostGhostedBoundaries()
{
  // No need to do this if using a serial mesh
  if (!_use_distributed_mesh)
    return;

  std::vector<dof_id_type> elems;
  std::vector<unsigned short int> sides;
  std::vector<boundary_id_type> ids;

  DistributedMesh & mesh = dynamic_cast<DistributedMesh &>(getMesh());

  mesh.clear_extra_ghost_elems();

  mesh.get_boundary_info().build_side_list(elems, sides, ids);

  std::set<const Elem *> boundary_elems_to_ghost;
  std::set<Node *> connected_nodes_to_ghost;

  std::vector<const Elem*> family_tree;

  for (unsigned int i = 0; i < elems.size(); ++i)
  {
    if (_ghosted_boundaries.find(ids[i]) != _ghosted_boundaries.end())
    {
      Elem * elem = mesh.elem_ptr(elems[i]);

#ifdef LIBMESH_ENABLE_AMR
      elem->family_tree(family_tree);
#else
      family_tree.clear();
      family_tree.push_back(elem);
#endif
      for (const auto & felem : family_tree)
      {
        boundary_elems_to_ghost.insert(felem);

        // The entries of connected_nodes_to_ghost need to be
        // non-constant, so that they will work in things like
        // UpdateDisplacedMeshThread.  Therefore, we are using the
        // "old" interface to get a non-const Node pointer from a
        // constant Elem.
        for (unsigned int n = 0; n < felem->n_nodes(); ++n)
          connected_nodes_to_ghost.insert (felem->get_node(n));
      }
    }
  }

  mesh.comm().allgather_packed_range(&mesh, connected_nodes_to_ghost.begin(), connected_nodes_to_ghost.end(), extra_ghost_elem_inserter<Node>(mesh));
  mesh.comm().allgather_packed_range(&mesh, boundary_elems_to_ghost.begin(), boundary_elems_to_ghost.end(), extra_ghost_elem_inserter<Elem>(mesh));
}

void
MooseMesh::setPatchSize(const unsigned int patch_size)
{
  _patch_size = patch_size;
}

unsigned int
MooseMesh::getPatchSize() const
{
  return _patch_size;
}

void
MooseMesh::setPatchUpdateStrategy(MooseEnum patch_update_strategy)
{
  _patch_update_strategy = patch_update_strategy;
}

const MooseEnum &
MooseMesh::getPatchUpdateStrategy() const
{
  return _patch_update_strategy;
}

MeshTools::BoundingBox
MooseMesh::getInflatedProcessorBoundingBox(Real inflation_multiplier) const
{
  // Grab a bounding box to speed things up
  MeshTools::BoundingBox bbox = MeshTools::processor_bounding_box(getMesh(), processor_id());

  // Inflate the bbox just a bit to deal with roundoff
  // Adding 1% of the diagonal size in each direction on each end
  Real inflation_amount = inflation_multiplier * (bbox.max() - bbox.min()).norm();
  Point inflation(inflation_amount, inflation_amount, inflation_amount);

  bbox.first -= inflation; // min
  bbox.second += inflation; // max

  return bbox;
}

MooseMesh::operator libMesh::MeshBase & ()
{
  return getMesh();
}

MooseMesh::operator const libMesh::MeshBase & () const
{
  return getMesh();
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

ExodusII_IO *
MooseMesh::exReader() const
{
  //TODO: Implement or remove
  return nullptr;
}

void MooseMesh::printInfo(std::ostream &os) const
{
  getMesh().print_info(os);
}

const std::vector<dof_id_type> &
MooseMesh::getNodeList(boundary_id_type nodeset_id) const
{
  std::map<boundary_id_type, std::vector<dof_id_type> >::const_iterator it = _node_set_nodes.find(nodeset_id);

  if (it == _node_set_nodes.end())
    mooseError("Unable to nodeset ID: " << nodeset_id << '.');

  return it->second;
}

const std::set<BoundaryID> &
MooseMesh::getSubdomainBoundaryIds(SubdomainID subdomain_id) const
{
  std::map<SubdomainID, std::set<BoundaryID> >::const_iterator it = _subdomain_boundary_ids.find(subdomain_id);

  if (it == _subdomain_boundary_ids.end())
    mooseError("Unable to find subdomain ID: " << subdomain_id << '.');

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
  std::map<boundary_id_type, std::set<dof_id_type> >::const_iterator it = _bnd_node_ids.find(bnd_id);
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
  std::map<boundary_id_type, std::set<dof_id_type> >::const_iterator it = _bnd_elem_ids.find(bnd_id);
  if (it != _bnd_elem_ids.end())
    if (it->second.find(elem_id) != it->second.end())
      found_elem = true;
  return found_elem;
}

void
MooseMesh::errorIfDistributedMesh(std::string name) const
{
  if (_use_distributed_mesh)
    mooseError("Cannot use " << name << " with DistributedMesh!\n"
               << "Consider specifying parallel_type = 'replicated' in your input file\n"
               << "to prevent it from being run with DistributedMesh.");
}

void
MooseMesh::errorIfParallelDistribution(std::string name) const
{
  mooseDeprecated("errorIfParallelDistribution() is deprecated, call errorIfDistributedMesh() instead.");
  errorIfDistributedMesh(name);
}


MooseMesh::MortarInterface *
MooseMesh::getMortarInterfaceByName(const std::string name)
{
  std::map<std::string, MortarInterface *>::iterator it = _mortar_interface_by_name.find(name);
  if (it != _mortar_interface_by_name.end())
    return (*it).second;
  else
    mooseError("Requesting non-existent mortar interface '" << name << "'.");
}

MooseMesh::MortarInterface *
MooseMesh::getMortarInterface(BoundaryID master, BoundaryID slave)
{
  std::map<std::pair<BoundaryID, BoundaryID>, MortarInterface *>::iterator it = _mortar_interface_by_ids.find(std::pair<BoundaryID, BoundaryID>(master, slave));
  if (it != _mortar_interface_by_ids.end())
    return (*it).second;
  else
    mooseError("Requesting non-existing mortar interface (master = " << master << ", slave = " << slave << ").");
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
MooseMesh::addMortarInterface(const std::string & name, BoundaryName master, BoundaryName slave, SubdomainName domain_name)
{
  SubdomainID domain_id = getSubdomainID(domain_name);
  boundary_id_type master_id = getBoundaryID(master);
  boundary_id_type slave_id = getBoundaryID(slave);

  std::unique_ptr<MortarInterface> iface = libmesh_make_unique<MortarInterface>();

  iface->_id = domain_id;
  iface->_master = master;
  iface->_slave = slave;
  iface->_name = name;

  MeshBase::element_iterator           el = _mesh->level_elements_begin(0);
  const MeshBase::element_iterator end_el = _mesh->level_elements_end(0);
  for (; el != end_el; ++el)
  {
    Elem * elem = *el;
    if (elem->subdomain_id() == domain_id)
      iface->_elems.push_back(elem);
  }

  setSubdomainName(iface->_id, name);

  _mortar_interface.push_back(std::move(iface));
  _mortar_interface_by_name[name] = _mortar_interface.back().get();
  _mortar_interface_by_ids[std::pair<BoundaryID, BoundaryID>(master_id, slave_id)] = _mortar_interface.back().get();
}
