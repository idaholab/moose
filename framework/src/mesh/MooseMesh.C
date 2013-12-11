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
#include "NonlinearSystem.h"
#include "CacheChangedListsThread.h"
#include "Assembly.h"
#include "MooseUtils.h"
#include "MooseApp.h"

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

static const int GRAIN_SIZE = 1;     // the grain_size does not have much influence on our execution speed

template<>
InputParameters validParams<MooseMesh>()
{
  InputParameters params = validParams<MooseObject>();

  MooseEnum mesh_distribution_type("PARALLEL=0, SERIAL, DEFAULT", "DEFAULT");
  params.addParam<MooseEnum>("distribution", mesh_distribution_type,
                             "PARALLEL: Always use libMesh::ParallelMesh "
                             "SERIAL: Always use libMesh::SerialMesh "
                             "DEFAULT: Use libMesh::SerialMesh unless --parallel-mesh is specified on the command line");

  params.addParam<bool>("nemesis", false,
                        "If nemesis=true and file=foo.e, actually reads "
                        "foo.e.N.0, foo.e.N.1, ... foo.e.N.N-1, "
                        "where N = # CPUs, with NemesisIO.");

  MooseEnum dims("1 = 1, 2, 3", "3");
  params.addParam<MooseEnum>("dim", dims,
                             "This is only required for certain mesh formats where "
                             "the dimension of the mesh cannot be autodetected.  "
                             "In particular you must supply this for GMSH meshes.  "
                             "Note: This is completely ignored for ExodusII meshes!");

  MooseEnum partitioning("default=-3, metis=-2, parmetis=-1, linear=0, centroid, hilbert_sfc, morton_sfc", "default");
  params.addParam<MooseEnum>("partitioner", partitioning, "Specifies a mesh partitioner to use when splitting the mesh for a parallel computation.");
  MooseEnum direction("x, y, z, radial");
  params.addParam<MooseEnum>("centroid_partitioner_direction", direction, "Specifies the sort direction if using the centroid partitioner. Available options: x, y, z, radial");

  params.addPrivateParam<std::string>("built_by_action", "setup_mesh");

  // groups
  params.addParamNamesToGroup("dim nemesis", "Advanced");
  params.addParamNamesToGroup("partitioner centroid_partitioner_direction", "Partitioning");

  return params;
}


MooseMesh::MooseMesh(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Restartable(name, parameters, "Mesh"),
    _mesh_distribution_type(getParam<MooseEnum>("distribution")),
    _use_parallel_mesh(false),
    _distribution_overridden(false),
    _mesh(NULL),
    _partitioner_name(getParam<MooseEnum>("partitioner")),
    _partitioner_overridden(false),
    _uniform_refine_level(0),
    _is_changed(false),
    _is_nemesis(getParam<bool>("nemesis")),
    _is_prepared(false),
    _refined_elements(NULL),
    _coarsened_elements(NULL),
    _active_local_elem_range(NULL),
    _active_semilocal_node_range(NULL),
    _active_node_range(NULL),
    _local_node_range(NULL),
    _bnd_node_range(NULL),
    _bnd_elem_range(NULL),
    _node_to_elem_map_built(false),
    _patch_size(40),
    _regular_orthogonal_mesh(false),
    _allow_recovery(true)
{
  switch (_mesh_distribution_type)
  {
  case 0: // PARALLEL
    _use_parallel_mesh = true;
    break;
  case 1: // SERIAL
    if (_app.getParallelMeshOnCommandLine() || _is_nemesis)
      _distribution_overridden = true;
    break;
  case 2: // DEFAULT
    // The user did not specify 'distribution = XYZ' in the input file,
    // so we allow the --parallel-mesh command line arg to possibly turn
    // on ParallelMesh.  If the command line arg is not present, we pick SerialMesh.
    if (_app.getParallelMeshOnCommandLine())
      _use_parallel_mesh = true;

    break;
    // No default switch needed for MooseEnum
  }

  // If the user specifies 'nemesis = true' in the Mesh block, we
  // must use ParallelMesh.
  if (_is_nemesis)
    _use_parallel_mesh = true;

  unsigned dim = getParam<MooseEnum>("dim");

  if (_use_parallel_mesh)
  {
    _mesh = new ParallelMesh(dim);
    if (_partitioner_name != "default" && _partitioner_name != "parmetis")
    {
      _partitioner_name = "parmetis";
      _partitioner_overridden = true;
    }
  }
  else
  {
    _mesh = new SerialMesh(dim);
  }

  // Set the partitioner
  switch (_partitioner_name)
  {
  case -3: // default
    // We'll use the default partitioner, but notify the user of which one is being used...
    if (_use_parallel_mesh)
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
    getMesh().partitioner() = AutoPtr<Partitioner>(new LinearPartitioner);
    break;
  case 1: // centroid
  {
    if (!isParamValid("centroid_partitioner_direction"))
      mooseError("If using the centroid partitioner you _must_ specify centroid_partitioner_direction!");

    MooseEnum direction = getParam<MooseEnum>("centroid_partitioner_direction");

    if(direction == "x")
      getMesh().partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::X));
    else if(direction == "y")
      getMesh().partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::Y));
    else if(direction == "z")
      getMesh().partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::Z));
    else if(direction == "radial")
      getMesh().partitioner() = AutoPtr<Partitioner>(new CentroidPartitioner(CentroidPartitioner::RADIAL));
    break;
  }
  case 2: // hilbert_sfc
    getMesh().partitioner() = AutoPtr<Partitioner>(new HilbertSFCPartitioner);
    break;
  case 3: // morton_sfc
    getMesh().partitioner() = AutoPtr<Partitioner>(new MortonSFCPartitioner);
    break;
  }
}

MooseMesh::MooseMesh(const MooseMesh & other_mesh) :
    MooseObject(other_mesh._name, other_mesh._pars),
    Restartable(_name, _pars, "Mesh"),
    _mesh_distribution_type(other_mesh._mesh_distribution_type),
    _use_parallel_mesh(other_mesh._use_parallel_mesh),
    _distribution_overridden(other_mesh._distribution_overridden),
    _mesh(other_mesh.getMesh().clone().release()),
    _partitioner_name(other_mesh._partitioner_name),
    _partitioner_overridden(other_mesh._partitioner_overridden),
    _uniform_refine_level(0),
    _is_changed(false),
    _is_nemesis(false),
    _is_prepared(false),
    _refined_elements(NULL),
    _coarsened_elements(NULL),
    _active_local_elem_range(NULL),
    _active_semilocal_node_range(NULL),
    _active_node_range(NULL),
    _local_node_range(NULL),
    _bnd_node_range(NULL),
    _bnd_elem_range(NULL),
    _node_to_elem_map_built(false),
    _patch_size(40),
    _regular_orthogonal_mesh(false)
{
  *(getMesh().boundary_info) = *(other_mesh.getMesh().boundary_info);

  const std::set<SubdomainID> & subdomains = other_mesh.meshSubdomains();
  for (std::set<SubdomainID>::const_iterator it = subdomains.begin(); it != subdomains.end(); ++it)
  {
    SubdomainID sid = *it;
    setSubdomainName(sid, other_mesh.getMesh().subdomain_name(sid));
  }

  std::vector<BoundaryID> side_boundaries;
  other_mesh.getMesh().boundary_info->build_side_boundary_ids(side_boundaries);
  for (std::vector<BoundaryID>::const_iterator it = side_boundaries.begin(); it != side_boundaries.end(); ++it)
  {
    BoundaryID bid = *it;
    getMesh().boundary_info->sideset_name(bid) = other_mesh.getMesh().boundary_info->sideset_name(bid);
  }

  std::vector<BoundaryID> node_boundaries;
  other_mesh.getMesh().boundary_info->build_node_boundary_ids(node_boundaries);
  for (std::vector<BoundaryID>::const_iterator it = node_boundaries.begin(); it != node_boundaries.end(); ++it)
  {
    BoundaryID bid = *it;
    getMesh().boundary_info->nodeset_name(bid) = other_mesh.getMesh().boundary_info->nodeset_name(bid);
  }
}

MooseMesh::~MooseMesh()
{
  freeBndNodes();
  freeBndElems();
  clearQuadratureNodes();

  delete _active_local_elem_range;
  delete _active_node_range;
  delete _active_semilocal_node_range;
  delete _local_node_range;
  delete _bnd_node_range;
  delete _bnd_elem_range;
  delete _refined_elements;
  delete _coarsened_elements;
  delete _mesh;
}

void
MooseMesh::freeBndNodes()
{
  // free memory
  for (std::vector<BndNode *>::iterator it = _bnd_nodes.begin(); it != _bnd_nodes.end(); ++it)
    delete (*it);

  for (std::map<short int, std::vector<unsigned int> >::iterator it = _node_set_nodes.begin(); it != _node_set_nodes.end(); ++it)
    it->second.clear();
  _node_set_nodes.clear();
  _bnd_node_ids.clear();
}

void
MooseMesh::freeBndElems()
{
  // free memory
  for (std::vector<BndElement *>::iterator it = _bnd_elems.begin(); it != _bnd_elems.end(); ++it)
    delete (*it);
}

void
MooseMesh::prepare(bool force)
{
  if (dynamic_cast<ParallelMesh *>(&getMesh()) && !_is_nemesis)
  {
    // Call prepare_for_use() and allow renumbering
    getMesh().allow_renumbering(true);
    if (force)
      getMesh().prepare_for_use();
  }
  else
  {
    // Call prepare_for_use() and DO NOT allow renumbering
    getMesh().allow_renumbering(false);
    if (force)
      getMesh().prepare_for_use();
  }

  // Collect (local) subdomain IDs
  const MeshBase::element_iterator el_end = getMesh().elements_end();

  for (MeshBase::element_iterator el = getMesh().elements_begin(); el != el_end; ++el)
    _mesh_subdomains.insert((*el)->subdomain_id());

  // Collect (local) boundary IDs
  const std::set<BoundaryID>& local_bids = getMesh().boundary_info->get_boundary_ids();
  _mesh_boundary_ids.insert(local_bids.begin(), local_bids.end());

  // Communicate subdomain and boundary IDs if this is a parallel mesh
  if (!getMesh().is_serial())
  {
    // Pack our subdomain IDs into a vector
    std::vector<SubdomainID> mesh_subdomains_vector(_mesh_subdomains.begin(),
                                                    _mesh_subdomains.end());

    // Gather them all into an enlarged vector
    Parallel::allgather(mesh_subdomains_vector);

    // Attempt to insert any new IDs into the set (any existing ones will be skipped)
    _mesh_subdomains.insert(mesh_subdomains_vector.begin(),
                            mesh_subdomains_vector.end());

    // Pack our boundary IDs into a vector for communication
    std::vector<BoundaryID> mesh_boundary_ids_vector(_mesh_boundary_ids.begin(),
                                                     _mesh_boundary_ids.end());

    // Gather them all into an enlarged vector
    Parallel::allgather(mesh_boundary_ids_vector);

    // Attempt to insert any new IDs into the set (any existing ones will be skipped)
    _mesh_boundary_ids.insert(mesh_boundary_ids_vector.begin(),
                              mesh_boundary_ids_vector.end());
  }

  detectOrthogonalDimRanges();

  update();

  // Prepared has been called
  _is_prepared = true;
}

void
MooseMesh::update()
{
  // Rebuild the boundary conditions
  buildNodeListFromSideList();

  //Update the node to elem map
  _node_to_elem_map.clear();
  _node_to_elem_map_built = false;

  buildNodeList();
  buildBndElemList();
  cacheInfo();
}

const Node &
MooseMesh::node(const unsigned int i) const
{
  if(i > getMesh().max_node_id())
    return *(*_quadrature_nodes.find(i)).second;

  return getMesh().node(i);
}

Node &
MooseMesh::node(const unsigned int i)
{
  if(i > getMesh().max_node_id())
    return *_quadrature_nodes[i];

  return getMesh().node(i);
}

const Node*
MooseMesh::nodePtr(const unsigned int i) const
{
  if(i > getMesh().max_node_id())
    return (*_quadrature_nodes.find(i)).second;

  return getMesh().node_ptr(i);
}

Node*
MooseMesh::nodePtr(const unsigned int i)
{
  if(i > getMesh().max_node_id())
    return _quadrature_nodes[i];

  return getMesh().node_ptr(i);
}

void
MooseMesh::meshChanged()
{
  update();

  // Rebuild the active local element range
  delete _active_local_elem_range;
  _active_local_elem_range = NULL;
  // Rebuild the node range
  delete _active_node_range;
  _active_node_range = NULL;
  // Rebuild the semilocal range
  delete _active_semilocal_node_range;
  _active_semilocal_node_range = NULL;
  // Rebuild the local node range
  delete _local_node_range;
  _local_node_range = NULL;
  // Rebuild the boundary node range
  delete _bnd_node_range;
  _bnd_node_range = NULL;

  // Rebuild the ranges
  getActiveLocalElementRange();
  getActiveNodeRange();
  getLocalNodeRange();
  getBoundaryNodeRange();

  // Lets the output system know that the mesh has changed recently.
  _is_changed = true;
}

void
MooseMesh::cacheChangedLists()
{
  ConstElemRange elem_range(getMesh().local_elements_begin(), getMesh().local_elements_end(), 1);
  CacheChangedListsThread cclt(*this);
  Threads::parallel_reduce(elem_range, cclt);

  delete _refined_elements;
  delete _coarsened_elements;
  _coarsened_element_children.clear();

  _refined_elements = new ConstElemPointerRange(cclt._refined_elements.begin(), cclt._refined_elements.end());
  _coarsened_elements = new ConstElemPointerRange(cclt._coarsened_elements.begin(), cclt._coarsened_elements.end());
  _coarsened_element_children = cclt._coarsened_element_children;
}

ConstElemPointerRange *
MooseMesh::refinedElementRange()
{
  return _refined_elements;
}

ConstElemPointerRange *
MooseMesh::coarsenedElementRange()
{
  return _coarsened_elements;
}

std::vector<const Elem *> &
MooseMesh::coarsenedElementChildren(const Elem * elem)
{
  return _coarsened_element_children[elem];
}

void
MooseMesh::updateActiveSemiLocalNodeRange(std::set<unsigned int> & ghosted_elems)
{
  _semilocal_node_list.clear();

  // First add the nodes connected to local elems
  ConstElemRange * active_local_elems = getActiveLocalElementRange();
  for(ConstElemRange::const_iterator it=active_local_elems->begin();
      it!=active_local_elems->end();
      ++it)
  {
    const Elem * elem = *it;
    for(unsigned int n=0; n<elem->n_nodes(); n++)
    {
      Node * node = elem->get_node(n);

      _semilocal_node_list.insert(node);
    }
  }

  // Now add the nodes connected to ghosted_elems
  for(std::set<unsigned int>::iterator it=ghosted_elems.begin();
      it!=ghosted_elems.end();
      ++it)
  {
    Elem * elem = getMesh().elem(*it);
    for(unsigned int n=0; n<elem->n_nodes(); n++)
    {
      Node * node = elem->get_node(n);

      _semilocal_node_list.insert(node);
    }
  }

  delete _active_semilocal_node_range;

  // Now create the actual range
  _active_semilocal_node_range = new SemiLocalNodeRange(_semilocal_node_list.begin(), _semilocal_node_list.end());
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
    if(lhs->_bnd_id < rhs->_bnd_id)
      return true;

    if(lhs->_bnd_id > rhs->_bnd_id)
      return false;

    if(lhs->_node->id() < rhs->_node->id())
      return true;

    if(lhs->_node->id() > rhs->_node->id())
      return false;

    return false;
  }
};

void
MooseMesh::buildNodeList()
{
  freeBndNodes();

  /// Boundary node list (node ids and corresponding side-set ids, arrays always have the same length)
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;
  getMesh().boundary_info->build_node_list(nodes, ids);

  int n = nodes.size();
  _bnd_nodes.resize(n);
  for (int i = 0; i < n; i++)
  {
    _bnd_nodes[i] = new BndNode(&getMesh().node(nodes[i]), ids[i]);
    _node_set_nodes[ids[i]].push_back(nodes[i]);
    _bnd_node_ids.insert(nodes[i]);
  }

  _bnd_nodes.reserve(_bnd_nodes.size() + _extra_bnd_nodes.size());
  for(unsigned int i=0; i<_extra_bnd_nodes.size(); i++)
  {
    BndNode * bnode = new BndNode(_extra_bnd_nodes[i]._node, _extra_bnd_nodes[i]._bnd_id);
    _bnd_nodes.push_back(bnode);
    _bnd_node_ids.insert(_extra_bnd_nodes[i]._node->id());
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
  std::vector<unsigned int> elems;
  std::vector<unsigned short int> sides;
  std::vector<boundary_id_type> ids;
  getMesh().boundary_info->build_side_list(elems, sides, ids);

  int n = elems.size();
  _bnd_elems.resize(n);
  for (int i = 0; i < n; i++)
  {
    _bnd_elems[i] = new BndElement(getMesh().elem(elems[i]), sides[i], ids[i]);
  }
}

std::map<unsigned int, std::vector<unsigned int> > &
MooseMesh::nodeToElemMap()
{
  if(!_node_to_elem_map_built)
  {
    _node_to_elem_map_built = true;
    MeshBase::const_element_iterator       el  = getMesh().elements_begin();
    const MeshBase::const_element_iterator end = getMesh().elements_end();

    for (; el != end; ++el)
      for (unsigned int n=0; n<(*el)->n_nodes(); n++)
        _node_to_elem_map[(*el)->node(n)].push_back((*el)->id());
  }

  return _node_to_elem_map;
}



ConstElemRange *
MooseMesh::getActiveLocalElementRange()
{
  if (!_active_local_elem_range)
  {
    _active_local_elem_range = new ConstElemRange(getMesh().active_local_elements_begin(),
                                                  getMesh().active_local_elements_end(), GRAIN_SIZE);
  }

  return _active_local_elem_range;
}

NodeRange *
MooseMesh::getActiveNodeRange()
{
  if (!_active_node_range)
  {
    _active_node_range = new NodeRange(getMesh().active_nodes_begin(),
                                       getMesh().active_nodes_end(), GRAIN_SIZE);
  }

  return _active_node_range;
}

SemiLocalNodeRange *
MooseMesh::getActiveSemiLocalNodeRange()
{
  mooseAssert(_active_semilocal_node_range, "_active_semilocal_node_range has not been created yet!");
/*
  if (!_active_node_range)
  {
    _active_semilocal_node_range = new NodeRange(getMesh().local_nodes_begin(),
                                                 getMesh().local_nodes_end(), GRAIN_SIZE);
  }
 */

  return _active_semilocal_node_range;
}

ConstNodeRange *
MooseMesh::getLocalNodeRange()
{
  if (!_local_node_range)
  {
    _local_node_range = new ConstNodeRange(getMesh().local_nodes_begin(),
                                           getMesh().local_nodes_end(), GRAIN_SIZE);
  }

  return _local_node_range;
}

ConstBndNodeRange *
MooseMesh::getBoundaryNodeRange()
{
  if (!_bnd_node_range)
  {
    _bnd_node_range = new ConstBndNodeRange(bndNodesBegin(),
                                            bndNodesEnd(), GRAIN_SIZE);
  }

  return _bnd_node_range;
}

ConstBndElemRange *
MooseMesh::getBoundaryElementRange()
{
  if (!_bnd_elem_range)
  {
    _bnd_elem_range = new ConstBndElemRange(bndElemsBegin(),
                                            bndElemsEnd(), GRAIN_SIZE);
  }

  return _bnd_elem_range;
}

void
MooseMesh::cacheInfo()
{
  const MeshBase::element_iterator end = getMesh().elements_end();
  for (MeshBase::element_iterator el = getMesh().elements_begin(); el != end; ++el)
  {
    Elem * elem = *el;

    unsigned int subdomain_id = elem->subdomain_id();

    for(unsigned int side=0; side<elem->n_sides(); side++)
    {
      std::vector<BoundaryID> boundaryids = boundaryIDs(elem, side);

      for(unsigned int i=0; i<boundaryids.size(); i++)
        _subdomain_boundary_ids[subdomain_id].insert(boundaryids[i]);
    }

    for(unsigned int nd = 0; nd < elem->n_nodes(); ++nd)
    {
      Node & node = *elem->get_node(nd);
      _block_node_list[node.id()].insert(elem->subdomain_id());
    }
  }
}

std::set<SubdomainID> &
MooseMesh::getNodeBlockIds(const Node & node)
{
  return _block_node_list[node.id()];
}


// default begin() accessor
bnd_node_iterator
MooseMesh::bndNodesBegin ()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.begin(), _bnd_nodes.end(), p);
}

// default end() accessor
bnd_node_iterator
MooseMesh::bndNodesEnd ()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.end(), _bnd_nodes.end(), p);
}

// default begin() accessor
bnd_elem_iterator
MooseMesh::bndElemsBegin ()
{
  Predicates::NotNull<bnd_elem_iterator_imp> p;
  return bnd_elem_iterator(_bnd_elems.begin(), _bnd_elems.end(), p);
}

// default end() accessor
bnd_elem_iterator
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
    for (libMesh::MeshBase::node_iterator i=getMesh().nodes_begin(); i != end; ++i)
    {
      _node_map.push_back(*i);
    }
  }

  Node *node = NULL;
  for (unsigned int i=0; i<_node_map.size(); ++i)
  {
    if (p.relative_fuzzy_equals(*_node_map[i], tol))
    {
      node = _node_map[i];
      break;
    }
  }
  if (node == NULL)
  {
    node = getMesh().add_node(new Node(p));
    _node_map.push_back(node);
  }

/*  Alternative method
  libMesh::MeshBase::node_iterator i = getMesh().nodes_begin();
  libMesh::MeshBase::node_iterator i_end = getMesh().nodes_end();
  Node *node = NULL;
  for (; i != i_end; ++i)
  {
    if (p.relative_fuzzy_equals(**i, tol))
    {
      node = *i;
      break;
    }
  }
  if (node == NULL)
    node = getMesh().add_node(new Node(p));
*/

  mooseAssert(node != NULL, "Node is NULL");
  return node;
}

Node *
MooseMesh::addQuadratureNode(const Elem * elem, const unsigned short int side, const unsigned int qp, BoundaryID bid, const Point & point)
{
  Node * qnode;

  if(_elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].find(qp) == _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side].end())
  {
    // Create a new node id starting from the max node id and counting down.  This will be the least
    // likely to collide with an existing node id.
    unsigned int max_id = std::numeric_limits<unsigned int>::max()-100;
    unsigned int new_id = max_id - _quadrature_nodes.size();

    if(new_id <= getMesh().max_node_id())
      mooseError("Quadrature node id collides with existing node id!");

    qnode = new Node(point, new_id);

    // Keep track of this new node in two different ways for easy lookup
    _quadrature_nodes[new_id] = qnode;
    _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side][qp] = qnode;

    _node_to_elem_map[new_id].push_back(elem->id());
  }
  else
    qnode = _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side][qp];

  BndNode * bnode = new BndNode(qnode, bid);
  _bnd_nodes.push_back(bnode);
  _bnd_node_ids.insert(qnode->id());

  _extra_bnd_nodes.push_back(*bnode);

  // Do this so the range will be regenerated next time it is accessed
  delete _bnd_node_range;
  _bnd_node_range = NULL;

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
  { // Delete all the quadrature nodes
    std::map<unsigned int, Node *>::iterator it = _quadrature_nodes.begin();
    std::map<unsigned int, Node *>::iterator end = _quadrature_nodes.end();

    for(; it != end; ++it)
      delete it->second;
  }

  _quadrature_nodes.clear();
  _elem_to_side_to_qp_to_quadrature_nodes.clear();
  _extra_bnd_nodes.clear();
}

BoundaryID
MooseMesh::getBoundaryID(const BoundaryName & boundary_name) const
{
  if (boundary_name == "ANY_BOUNDARY_ID")
    mooseError("Please use getBoundaryIDs() when passing \"ANY_BOUNDARY_ID\"");

  BoundaryID id;
  std::istringstream ss(boundary_name);

  if (!(ss >> id))
    id = getMesh().boundary_info->get_id_by_name(boundary_name);

  return id;
}

std::vector<BoundaryID>
MooseMesh::getBoundaryIDs(const std::vector<BoundaryName> & boundary_name, bool generate_unknown) const
{
  std::vector<BoundaryID> ids(boundary_name.size());
  std::map<BoundaryID, std::string> & sideset_map = getMesh().boundary_info->set_sideset_name_map();
  std::map<BoundaryID, std::string> & nodeset_map = getMesh().boundary_info->set_nodeset_name_map();
  std::set<BoundaryID> boundary_ids = getMesh().boundary_info->get_boundary_ids();
  BoundaryID max_boundary_id = boundary_ids.empty() ? 0 : *(getMesh().boundary_info->get_boundary_ids().rbegin());

  for(unsigned int i=0; i<boundary_name.size(); i++)
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
       * If the converstion from a name to a number fails, that means that this must be a named
       * boundary.  We will look in the complete map for this sideset and create a new name/ID pair
       * if requested.
       */
      if (generate_unknown
          && !MooseUtils::doesMapContainValue(sideset_map, std::string(boundary_name[i]))
          && !MooseUtils::doesMapContainValue(nodeset_map, std::string(boundary_name[i])))
        id = ++max_boundary_id;
      else
        id = getMesh().boundary_info->get_id_by_name(boundary_name[i]);
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

  SubdomainID id;
  std::istringstream ss(subdomain_name);

  if (!(ss >> id))
    id = getMesh().get_id_by_name(subdomain_name);

  return id;
}

std::vector<SubdomainID>
MooseMesh::getSubdomainIDs(const std::vector<SubdomainName> & subdomain_name) const
{
  std::vector<SubdomainID> ids(subdomain_name.size());

  for(unsigned int i=0; i<subdomain_name.size(); i++)
  {
    if (subdomain_name[i] == "ANY_BLOCK_ID")
    {
      ids.assign(_mesh_subdomains.begin(), _mesh_subdomains.end());
      if (i)
        mooseWarning("You passed \"ANY_BLOCK_ID\" in addition to other sudomain_names.  This may be a logic error.");
      break;
    }

    SubdomainID id;
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
  std::vector<BoundaryID> side_boundaries;
  getMesh().boundary_info->build_side_boundary_ids(side_boundaries);

  // We need to figure out if this boundary is a sideset or nodeset
  if (std::find(side_boundaries.begin(), side_boundaries.end(), boundary_id) != side_boundaries.end())
    getMesh().boundary_info->sideset_name(boundary_id) = name;
  else
    getMesh().boundary_info->nodeset_name(boundary_id) = name;
}

void
MooseMesh::buildPeriodicNodeMap(std::multimap<unsigned int, unsigned int> & periodic_node_map, unsigned int var_number, PeriodicBoundaries *pbs) const
{
  mooseAssert(!Threads::in_threads, "This function should only be called outside of a threaded region due to the use of PointLocator");

  periodic_node_map.clear();

  MeshBase::const_element_iterator it = getMesh().active_local_elements_begin();
  MeshBase::const_element_iterator it_end = getMesh().active_local_elements_end();
  AutoPtr<PointLocatorBase> point_locator = getMesh().sub_point_locator();

  for (; it != it_end; ++it)
  {
    const Elem *elem = *it;
    for (unsigned int s=0; s<elem->n_sides(); ++s)
    {
      if (elem->neighbor(s))
        continue;

      const std::vector<boundary_id_type>& bc_ids = getMesh().boundary_info->boundary_ids (elem, s);
      for (std::vector<boundary_id_type>::const_iterator id_it = bc_ids.begin(); id_it!=bc_ids.end(); ++id_it)
      {
        const boundary_id_type boundary_id = *id_it;
        const PeriodicBoundaryBase *periodic = pbs->boundary(boundary_id);
        if (periodic && periodic->is_my_variable(var_number))
        {
          const Elem* neigh = pbs->neighbor(boundary_id, *point_locator, elem, s);
          unsigned int s_neigh = getMesh().boundary_info->side_with_boundary_id (neigh, periodic->pairedboundary);

          AutoPtr<Elem> elem_side = elem->build_side(s);
          AutoPtr<Elem> neigh_side = neigh->build_side(s_neigh);

          // At this point we have matching sides - lets find matching nodes
          for (unsigned int i=0; i<elem_side->n_nodes(); ++i)
          {
            Node *master_node = elem->get_node(i);
            Point master_point = periodic->get_corresponding_pos(*master_node);
            for (unsigned int j=0; j<neigh_side->n_nodes(); ++j)
            {
              Node *slave_node = neigh_side->get_node(j);
              if (master_point.absolute_fuzzy_equals(*slave_node))
              {
                // Avoid inserting any duplicates
                std::pair<std::multimap<unsigned int, unsigned int>::iterator, std::multimap<unsigned int, unsigned int>::iterator> iters =
                  periodic_node_map.equal_range(master_node->id());
                bool found = false;
                for (std::multimap<unsigned int, unsigned int>::iterator map_it = iters.first; map_it != iters.second; ++map_it)
                  if (map_it->second == slave_node->id())
                    found = true;
                if (!found)
                  periodic_node_map.insert(std::make_pair(master_node->id(), slave_node->id()));
              }
            }
          }
        }
      }
    }
  }
}

void
MooseMesh::buildPeriodicNodeSets(std::map<BoundaryID, std::set<unsigned int> > & periodic_node_sets, unsigned int var_number, PeriodicBoundaries *pbs) const
{
  periodic_node_sets.clear();

  std::vector<unsigned int> nl;
  std::vector<boundary_id_type> il;

  getMesh().boundary_info->build_node_list(nl, il);

  // Loop over all the boundary nodes adding the periodic nodes to the appropriate set
  for (unsigned int i=0; i<nl.size(); ++i)
  {
    // Is this current node on a known periodic boundary?
    if (periodic_node_sets.find(il[i]) != periodic_node_sets.end())
      periodic_node_sets[il[i]].insert(nl[i]);
    else // This still might be a periodic node but we just haven't seen this boundary_id yet
    {
      const PeriodicBoundaryBase *periodic = pbs->boundary(il[i]);
      if (periodic && periodic->is_my_variable(var_number))
        periodic_node_sets[il[i]].insert(nl[i]);
    }
  }
}

bool
MooseMesh::detectOrthogonalDimRanges(Real tol)
{
  std::vector<Real> min(3, std::numeric_limits<Real>::max());
  std::vector<Real> max(3, std::numeric_limits<Real>::min());
  unsigned int dim = getMesh().mesh_dimension();

  // Find the bounding box of our mesh
  const MeshBase::node_iterator nd_end = getMesh().nodes_end();
  for (MeshBase::node_iterator nd = getMesh().nodes_begin(); nd != nd_end; ++nd)
  {
    Node &node = **nd;
    for (unsigned int i=0; i<dim; ++i)
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
    Node &node = **nd;
    unsigned int coord_match = 0;

    for (unsigned int i=0; i<dim; ++i)
    {
      if (std::abs(node(i) - min[i]) < tol)
      {
        comp_map[i] = MIN;
        ++coord_match;
      }
      else if (std::abs(node(i) - max[i]) <tol)
      {
        comp_map[i] = MAX;
        ++coord_match;
      }
    }

    if (coord_match == dim)  // Found a coordinate at one of the extremes
    {
      _extreme_nodes[comp_map[X]*4 + comp_map[Y]*2 + comp_map[Z]] = &node;
      ++extreme_matches;
    }
  }

  // See if we matched all of the extremes for the mesh dimension
  if (extreme_matches != std::pow(2.0, (int)dim))
    return false;                    // This is not a regular orthogonal mesh

  // This is a regular orthogonal mesh, so set the bounds
  _regular_orthogonal_mesh = true;
  _bounds.resize(LIBMESH_DIM);
  for (unsigned int i=0; i<dim; ++i)
  {
    _bounds[i].resize(2);
    _bounds[i][MIN] = min[i];
    _bounds[i][MAX] = max[i];
  }
  for (unsigned int i=dim; i<LIBMESH_DIM; ++i)
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
  /**
   * Autodetect sidesets:  We need to inspect all of the nodes on the bounding box of each dimension to find the common
   * sideset.  This is rather tricky in higher dimensions.  We use the strides array below to help us index into the
   * corner nodes vector with the proper pattern.  To find the common boundary we simply locate the
   * boundary with the most unique hits (common to all of the nodes on that particular boundary).
   */
  unsigned int strides[3][3] = {{4, 2, 1}, {2, 4, 1}, {1, 4, 2}};

  getMesh().boundary_info->build_node_list_from_side_list();
  unsigned int dim = getMesh().mesh_dimension();
  unsigned int z_dim = dim == 3 ? 2 : 1;  // Determine how many z loops there are, we always need to loop at least once!
  for (unsigned int curr_dim=0; curr_dim < dim; ++curr_dim)
  {
    std::pair<BoundaryID, BoundaryID> paired_boundary;
    for (unsigned int i=0; i<2; ++i)
    {
      std::map<BoundaryID, unsigned int> boundary_counts;
      for (unsigned int j=0; j<2; ++j)
        for (unsigned int k=0; k<z_dim; ++k)
        {
          std::vector<BoundaryID> bounds_ids = getMesh().boundary_info->boundary_ids(_extreme_nodes[i*strides[curr_dim][0] + j*strides[curr_dim][1] + k*strides[curr_dim][2]]);
          for (unsigned int l=0; l<bounds_ids.size(); ++l)
            ++boundary_counts[bounds_ids[l]];
        }

      BoundaryID common_boundary;
      unsigned int max_count = 0;
      for (std::map<BoundaryID, unsigned int>::const_iterator it=boundary_counts.begin(); it != boundary_counts.end(); ++it)
      {
        if (it->second > max_count)
        {
          common_boundary = it->first;
          max_count = it->second;
        }
      }

      // If this test fails, it means that the autodetection failed.  We'll just exit gracefully from this routine.
      // Note, this means that the _paired_boundary datastructure will not be populated
      if (max_count < std::pow(2.0, (int)dim - 1))
        return;

      if (i==0)
        paired_boundary.first = common_boundary;
      else
        paired_boundary.second = common_boundary;
    }
    _paired_boundary.push_back(paired_boundary);
  }
}

Real
MooseMesh::dimensionWidth(unsigned int component) const
{
  mooseAssert(_regular_orthogonal_mesh, "The current mesh is not a regular orthogonal mesh");
  mooseAssert(component < LIBMESH_DIM, "Requested dimension out of bounds");

  return _bounds[component][MAX] - _bounds[component][MIN];
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

  _half_range = Point(dimensionWidth(0)/2.0, dimensionWidth(1)/2.0, dimensionWidth(2)/2.0);

  for (unsigned int component=0; component<dimension(); ++component)
  {
    std::pair<BoundaryID, BoundaryID> *boundary_ids = getPairedBoundaryMapping(component);

    if (boundary_ids != NULL &&
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

Real
MooseMesh::minPeriodicDistance(unsigned int nonlinear_var_num, Point p, Point q) const
{
  for (unsigned int i=0; i<dimension(); ++i)
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

  return (p-q).size();
}

std::pair<BoundaryID, BoundaryID> *
MooseMesh::getPairedBoundaryMapping(unsigned int component)
{
  mooseAssert(_regular_orthogonal_mesh, "The current mesh is not a regular orthogonal mesh");
  mooseAssert(component < dimension(), "Requested dimension out of bounds");

  if (_paired_boundary.empty())
  {
    if (_regular_orthogonal_mesh)
      detectPairedSidesets();
    else
      mooseError("Trying to retrieve automatic paired mapping for a mesh that is not regular and orthogonal");
  }

  if (component < _paired_boundary.size())
    return &_paired_boundary[component];
  else
    return NULL;
}

void
MooseMesh::buildRefinementAndCoarseningMaps(Assembly * assembly)
{
  MeshBase::const_element_iterator       el     = getMesh().elements_begin();
  const MeshBase::const_element_iterator end_el = getMesh().elements_end();

  std::map<ElemType, Elem *> canonical_elems;

  // First, loop over all elements and find a canonical element for each type
  // Doing it this way guarantees that this is going to work in parallel
  for ( ; el != end_el ; ++el) // TODO: Thread this
  {
    Elem * elem = *el;
    ElemType type = elem->type();

    if(canonical_elems.find(type) == canonical_elems.end()) // If we haven't seen this type of elem before save it
      canonical_elems[type] = elem;
    else
    {
      Elem * stored = canonical_elems[type];
      if(elem->id() < stored->id()) // Arbitrarily keep the one with a lower id
        canonical_elems[type] = elem;
    }
  }
  // Now build the maps using these templates
  // Note: This MUST be done NOT threaded!
  for(std::map<ElemType, Elem *>::iterator can_it = canonical_elems.begin();
      can_it != canonical_elems.end();
      ++can_it)
  {
    Elem * elem = can_it->second;

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
    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      // Side to side for sides that match parent's sides
      buildRefinementMap(*elem, *qrule, *qrule_face, side, -1, side);
      buildCoarseningMap(*elem, *qrule, *qrule_face, side);
    }

    // Child side to parent volume mapping for "internal" child sides
    for(unsigned int child=0; child<elem->n_children(); child++)
      for(unsigned int side=0; side<elem->n_sides(); side++) // Assume children have the same number of sides!
        if(!elem->is_child_on_side(child, side)) // Otherwise we already computed that map
          buildRefinementMap(*elem, *qrule, *qrule_face, -1, child, side);
  }
}

void
MooseMesh::buildRefinementMap(const Elem & elem, QBase & qrule, QBase & qrule_face, int parent_side, int child, int child_side)
{
  if(child == -1) // Doing volume mapping or parent side mapping
  {
    mooseAssert(parent_side == child_side, "Parent side must match child_side if not passing a specific child!");

    std::pair<int, ElemType> the_pair(parent_side, elem.type());

    if(_elem_type_to_refinement_map.find(the_pair) != _elem_type_to_refinement_map.end())
      mooseError("Already built a qp refinement map!");

    std::vector<std::pair<unsigned int, QpMap> > coarsen_map;
    std::vector<std::vector<QpMap> > & refinement_map = _elem_type_to_refinement_map[the_pair];
    findAdaptivityQpMaps(&elem, qrule, qrule_face, refinement_map, coarsen_map, parent_side, child, child_side);
  }
  else // Need to map a child side to parent volume qps
  {
    std::pair<int, int> child_pair(child, child_side);

    if(_elem_type_to_child_side_refinement_map.find(elem.type()) != _elem_type_to_child_side_refinement_map.end() &&
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
  if(child == -1) // Doing volume mapping or parent side mapping
  {
    mooseAssert(parent_side == child_side, "Parent side must match child_side if not passing a specific child!");

    std::pair<int, ElemType> the_pair(parent_side, elem.type());

    if(_elem_type_to_refinement_map.find(the_pair) == _elem_type_to_refinement_map.end())
      mooseError("Could not find a suitable qp refinement map!");

    return _elem_type_to_refinement_map[the_pair];
  }
  else // Need to map a child side to parent volume qps
  {
    std::pair<int, int> child_pair(child, child_side);

    if(_elem_type_to_child_side_refinement_map.find(elem.type()) == _elem_type_to_child_side_refinement_map.end() ||
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

  if(_elem_type_to_coarsening_map.find(the_pair) != _elem_type_to_coarsening_map.end())
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

  if(_elem_type_to_coarsening_map.find(the_pair) == _elem_type_to_coarsening_map.end())
    mooseError("Could not find a suitable qp refinement map!");

  return _elem_type_to_coarsening_map[the_pair];
}

void
MooseMesh::mapPoints(const std::vector<Point> & from, const std::vector<Point> & to, std::vector<QpMap> & qp_map)
{
  unsigned int n_from = from.size();
  unsigned int n_to = to.size();

  qp_map.resize(n_from);

  for(unsigned int i=0; i<n_from; i++)
  {
    const Point & from_point = from[i];

    QpMap & current_map = qp_map[i];

    for(unsigned int j=0; j<n_to; j++)
    {
      const Point & to_point = to[j];
      Real distance = (from_point - to_point).size();

      if(distance < current_map._distance)
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
  SerialMesh mesh;
  mesh.skip_partitioning(true);

  unsigned int dim = template_elem->dim();
  mesh.set_mesh_dimension(dim);

  for(unsigned int i=0; i<template_elem->n_nodes(); i++)
    mesh.add_point(template_elem->point(i));

  Elem * elem = mesh.add_elem(Elem::build(template_elem->type()).release());

  for(unsigned int i=0; i<template_elem->n_nodes(); i++)
    elem->set_node(i) = mesh.node_ptr(i);

  AutoPtr<FEBase> fe (FEBase::build(dim, FEType()));
  fe->get_phi();
  const std::vector<Point>& q_points_volume = fe->get_xyz();

  AutoPtr<FEBase> fe_face (FEBase::build(dim, FEType()));
  fe_face->get_phi();
  const std::vector<Point>& q_points_face = fe_face->get_xyz();

  fe->attach_quadrature_rule (&qrule);
  fe_face->attach_quadrature_rule (&qrule_face);

  // The current q_points
  const std::vector<Point> * q_points;

  if(parent_side != -1)
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

  if(child != -1) // Passed in a child explicitly
    children.push_back(child);
  else
  {
    children.resize(n_children);
    for(unsigned int child=0; child < n_children; child++)
      children[child] = child;
  }

  for(unsigned int i=0; i < children.size(); i++)
  {
    unsigned int child = children[i];

    if((parent_side != -1 && !elem->is_child_on_side(child, parent_side)))
      continue;

    const Elem * child_elem = elem->child(child);

    if(child_side != -1)
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
  for(unsigned int child=0; child < n_children; child++)
  {
    if(parent_side != -1 && !elem->is_child_on_side(child, child_side))
      continue;

    std::vector<Point> & child_ref_points = child_to_ref_points[child];

    std::vector<QpMap> qp_map;

    // Find all of the closest points from parent_qp to _THIS_ child's qp
    mapPoints(parent_ref_points, child_ref_points, qp_map);

    // Check those to see if they are closer than what we currently have for each point
    for(unsigned int parent_qp=0; parent_qp < parent_ref_points.size(); parent_qp++)
    {
      std::pair<unsigned int, QpMap> & child_and_map = coarsen_map[parent_qp];
      unsigned int & closest_child = child_and_map.first;
      QpMap & closest_map = child_and_map.second;

      QpMap & current_map = qp_map[parent_qp];

      if(current_map._distance < closest_map._distance)
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
  // Only level-0 elements store BCs.  Loop over them.
  MeshBase::element_iterator           el = getMesh().level_elements_begin(0);
  const MeshBase::element_iterator end_el = getMesh().level_elements_end(0);
  for (; el != end_el; ++el)
  {
    Elem *elem = *el;
    unsigned int n_sides = elem->n_sides();
    for (unsigned int s=0; s != n_sides; ++s)
    {
      const std::vector<boundary_id_type>& old_ids = getMesh().boundary_info->boundary_ids(elem, s);
      if (std::find(old_ids.begin(), old_ids.end(), old_id) != old_ids.end())
      {
        std::vector<boundary_id_type> new_ids(old_ids);
        std::replace(new_ids.begin(), new_ids.end(), old_id, new_id);
        if (delete_prev)
        {
          getMesh().boundary_info->remove_side(elem, s);
          getMesh().boundary_info->add_side(elem, s, new_ids);
        }
        else
        {
          getMesh().boundary_info->add_side(elem, s, new_ids);
        }
      }
    }
  }
}

const RealVectorValue &
MooseMesh::getNormalByBoundaryID(BoundaryID id) const
{
  mooseAssert(_boundary_to_normal_map.get() != NULL, "Boundary To Normal Map not built!");

  // Note: Boundaries that are not in the map (existing boundaries) will default
  // construct a new RealVectorValue - (x,y,z)=(0, 0, 0)
  return (*_boundary_to_normal_map)[id];
}

void
MooseMesh::init()
{
  if(!_app.isRecovering() || !_allow_recovery)
    buildMesh();
  else // When recovering just read the CPR file
    getMesh().read(_app.getRecoverFileBase() + "_mesh.cpr");
}

unsigned int
MooseMesh::dimension() const
{
  return getMesh().mesh_dimension();
}

std::vector<BoundaryID>
MooseMesh::boundaryIDs(const Elem *const elem, const unsigned short int side) const
{
  return getMesh().boundary_info->boundary_ids(elem, side);
}

const std::set<BoundaryID> &
MooseMesh::getBoundaryIDs() const
{
  return getMesh().boundary_info->get_boundary_ids();
}

void
MooseMesh::buildNodeListFromSideList()
{
  getMesh().boundary_info->build_node_list_from_side_list();
}

void
MooseMesh::buildSideList(std::vector<unsigned int> & el, std::vector<unsigned short int> & sl, std::vector<short int> & il)
{
  getMesh().boundary_info->build_side_list(el, sl, il);
}

unsigned int
MooseMesh::sideWithBoundaryID(const Elem * const elem, const BoundaryID boundary_id) const
{
  return getMesh().boundary_info->side_with_boundary_id(elem, boundary_id);
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

unsigned int
MooseMesh::nNodes() const
{
  return getMesh().n_nodes();
}

unsigned int
MooseMesh::nElem() const
{
  return getMesh().n_elem();
}

Elem *
MooseMesh::elem(const unsigned int i)
{
  return getMesh().elem(i);
}

const Elem *
MooseMesh::elem(const unsigned int i) const
{
  return getMesh().elem(i);
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

#ifdef LIBMESH_ENABLE_AMR
unsigned int & MooseMesh::uniformRefineLevel()
{
  return _uniform_refine_level;
}
#endif // LIBMESH_ENABLE_AMR

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

std::set<unsigned int> &
MooseMesh::getGhostedBoundaries()
{
  return _ghosted_boundaries;
}

std::vector<Real> &
MooseMesh::getGhostedBoundaryInflation()
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
  struct extra_ghost_elem_inserter
    : std::iterator<std::output_iterator_tag, T>
  {
    extra_ghost_elem_inserter(ParallelMesh& m) : mesh(m) {}

    void operator=(Elem* e) { mesh.add_extra_ghost_elem(e); }

    void operator=(Node* n) { mesh.add_node(n); }

    void operator=(Point* p) { mesh.add_point(*p); }

    extra_ghost_elem_inserter& operator++() {
      return *this;
    }

    extra_ghost_elem_inserter operator++(int) {
      return extra_ghost_elem_inserter(*this);
    }

    // We don't return a reference-to-T here because we don't want to
    // construct one or have any of its methods called.  We just want
    // to allow the returned object to be able to do mesh insertions
    // with operator=().
    extra_ghost_elem_inserter& operator*() { return *this; }
  private:

    ParallelMesh& mesh;
  };

} // anonymous namespace


void
MooseMesh::ghostGhostedBoundaries()
{
  // No need to do this if using a serial mesh
  if(!_use_parallel_mesh)
    return;

  Moose::perf_log.push("ghostGhostedBoundaries()","MooseMesh");

  std::vector<unsigned int> elems;
  std::vector<unsigned short int> sides;
  std::vector<boundary_id_type> ids;

  ParallelMesh & mesh = dynamic_cast<ParallelMesh &>(getMesh());

  mesh.clear_extra_ghost_elems();

  mesh.boundary_info->build_side_list(elems, sides, ids);

  std::set<const Elem *> boundary_elems_to_ghost;
  std::set<Node *> connected_nodes_to_ghost;

  std::vector<const Elem*> family_tree;

  for(unsigned int i=0; i<elems.size(); i++)
  {
    if(_ghosted_boundaries.find(ids[i]) != _ghosted_boundaries.end())
    {
      Elem * elem = mesh.elem(elems[i]);


#ifdef LIBMESH_ENABLE_AMR
      elem->family_tree(family_tree);
#else
      family_tree.clear();
      family_tree.push_back(elem);
#endif
      for (unsigned int leaf=0; leaf<family_tree.size(); leaf++)
      {
        const Elem * felem = family_tree[leaf];
        boundary_elems_to_ghost.insert(felem);

        for (unsigned int n=0; n<felem->n_nodes(); n++)
          connected_nodes_to_ghost.insert (felem->get_node(n));
      }
    }
  }

  mesh.comm().allgather_packed_range(&mesh, connected_nodes_to_ghost.begin(), connected_nodes_to_ghost.end(), extra_ghost_elem_inserter<Node>(mesh));
  mesh.comm().allgather_packed_range(&mesh, boundary_elems_to_ghost.begin(), boundary_elems_to_ghost.end(), extra_ghost_elem_inserter<Elem>(mesh));

  Moose::perf_log.pop("ghostGhostedBoundaries()","MooseMesh");
}

void
MooseMesh::setPatchSize(const unsigned int patch_size)
{
  _patch_size = patch_size;
}

unsigned int
MooseMesh::getPatchSize()
{
  return _patch_size;
}

MooseMesh::operator libMesh::MeshBase &()
{
  return getMesh();
}

MeshBase &
MooseMesh::getMesh()
{
  return *_mesh;
}

const MeshBase &
MooseMesh::getMesh() const
{
  return *_mesh;
}

ExodusII_IO *
MooseMesh::exReader() const
{
  return NULL;
}

void MooseMesh::printInfo(std::ostream &os)
{
  getMesh().print_info(os);
}

std::vector<unsigned int> &
MooseMesh::getNodeList(short int nodeset_id)
{
  return _node_set_nodes[nodeset_id];
}

const std::set<unsigned int> &
MooseMesh::getSubdomainBoundaryIds(unsigned int subdomain_id)
{
  return _subdomain_boundary_ids[subdomain_id];
}

bool
MooseMesh::isBoundaryNode(unsigned int node_id)
{
  return _bnd_node_ids.find(node_id) != _bnd_node_ids.end();
}

void
MooseMesh::errorIfParallelDistribution(std::string name) const
{
  if (_use_parallel_mesh)
    mooseError("Cannot use " << name << " with ParallelMesh!\n"
               << "Consider specifying distribution = 'serial' in your input file\n"
               << "to prevent it from being run with ParallelMesh.");
}

