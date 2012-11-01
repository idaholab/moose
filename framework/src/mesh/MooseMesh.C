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
#include "MeshModifier.h"

// libMesh
#include "boundary_info.h"
#include "mesh_tools.h"
#include "parallel.h"
#include "mesh_communication.h"
#include "parallel_mesh.h"
#include "periodic_boundary_base.h"

static const int GRAIN_SIZE = 1;     // the grain_size does not have much influence on our execution speed

template<>
InputParameters validParams<MooseMesh>()
{
  InputParameters params = validParams<MooseObject>();

  params.addParam<MeshFileName>("file", "The name of the mesh file to read (required unless using dynamic generation)");
  params.addParam<bool>("nemesis", false, "If nemesis=true and file=foo.e, actually reads foo.e.N.0, foo.e.N.1, ... foo.e.N.N-1, where N = # CPUs, with NemesisIO.");
  params.addPrivateParam<std::string>("built_by_action", "read_mesh");

  MooseEnum dims("1 = 1, 2, 3", "3");
  params.addParam<MooseEnum>("dim", dims, "This is only required for certain mesh formats where the dimension of the mesh cannot be autodetected.  In particular you must supply this for GMSH meshes.  Note: This is completely ignored for ExodusII meshes!");

  // groups
  params.addParamNamesToGroup("nemesis dim", "Advanced");

  return params;
}


MooseMesh::MooseMesh(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    _mesh(getParam<MooseEnum>("dim")),
    _is_changed(false),
    _is_parallel(getParam<bool>("nemesis")),
    _active_local_elem_range(NULL),
    _active_semilocal_node_range(NULL),
    _active_node_range(NULL),
    _local_node_range(NULL),
    _bnd_node_range(NULL),
    _bnd_elem_range(NULL),
    _node_to_elem_map_built(false),
    _patch_size(40)
{
}

MooseMesh::MooseMesh(const MooseMesh & other_mesh) :
    MooseObject(other_mesh._name, other_mesh._pars),
    _mesh(other_mesh._mesh),
    _is_changed(false),
    _is_parallel(getParam<bool>("nemesis")),
    _active_local_elem_range(NULL),
    _active_semilocal_node_range(NULL),
    _active_node_range(NULL),
    _local_node_range(NULL),
    _bnd_node_range(NULL),
    _bnd_elem_range(NULL),
    _node_to_elem_map_built(false),
    _patch_size(40)
{
  (*_mesh.boundary_info) = (*other_mesh._mesh.boundary_info);
}

MooseMesh::~MooseMesh()
{
  freeBndNodes();
  freeBndElems();

  delete _active_local_elem_range;
  delete _active_node_range;
  delete _active_semilocal_node_range;
  delete _local_node_range;
  delete _bnd_node_range;
  delete _bnd_elem_range;
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
}

void
MooseMesh::freeBndElems()
{
  // free memory
  for (std::vector<BndElement *>::iterator it = _bnd_elems.begin(); it != _bnd_elems.end(); ++it)
    delete (*it);
}

void
MooseMesh::read(const std::string file_name)
{
  if (dynamic_cast<ParallelMesh *>(&_mesh) && !_is_parallel)
    _mesh.read(file_name, NULL, false);
  else
    _mesh.read(file_name, NULL, true);
}

void
MooseMesh::prepare()
{
  // This is now done in Nemesis_IO::read()
  // If we are using a truly Parallel mesh (like Nemesis) then we might not even have neighbors!
//  if(parallel())
//    MeshCommunication().gather_neighboring_elements(libmesh_cast_ref<ParallelMesh&>(getMesh()));
//
  if (dynamic_cast<ParallelMesh *>(&_mesh) && !_is_parallel)
  {
    _mesh.allow_renumbering(true);
    _mesh.prepare_for_use(false);
  }
  else
  {
    _mesh.allow_renumbering(false);
    _mesh.prepare_for_use(true);
  }

  // If using ParallelMesh this will delete non-local elements from the current processor
  // If using SerialMesh, this function is a no-op.
  _mesh.delete_remote_elements();

//  if(!_mesh.is_serial())
//    Moose::gatherNearbyElements(*this, _ghosted_boundaries, _ghosted_boundaries_inflation);

  // Collect (local) subdomain IDs
  const MeshBase::element_iterator el_end = _mesh.elements_end();
/*
  unsigned int num_elems = 0;
  for (MeshBase::element_iterator el = _mesh.elements_begin(); el != el_end; ++el)
    num_elems++;

//  std::cerr<<libMesh::processor_id()<<": num_elems: "<<num_elems<<std::endl;

  Parallel::sum(num_elems);

  std::cout<<"Total elems: "<<num_elems<<std::endl;
*/

  for (MeshBase::element_iterator el = _mesh.elements_begin(); el != el_end; ++el)
    _mesh_subdomains.insert((*el)->subdomain_id());

  // Collect (local) boundary IDs
  const std::set<BoundaryID>& local_bids = _mesh.boundary_info->get_boundary_ids();
  _mesh_boundary_ids.insert(local_bids.begin(), local_bids.end());

  // Communicate subdomain and boundary IDs if this is a parallel mesh
  if (!_mesh.is_serial())
    {
      // Subdomain size before
      // std::cout << "(before) _mesh_subdomains.size()=" << _mesh_subdomains.size() << std::endl;

      // Pack our subdomain IDs into a vector
      std::vector<SubdomainID> mesh_subdomains_vector(_mesh_subdomains.begin(),
                                                      _mesh_subdomains.end());

      // Gather them all into an enlarged vector
      Parallel::allgather(mesh_subdomains_vector);

      // Attempt to insert any new IDs into the set (any existing ones will be skipped)
      _mesh_subdomains.insert(mesh_subdomains_vector.begin(),
                              mesh_subdomains_vector.end());

      // Subdomain size after
      // std::cout << "(after) _mesh_subdomains.size()=" << _mesh_subdomains.size() << std::endl;




      // Boundary ID size before
      // std::cout << "(before) _mesh_boundary_ids.size()=" << _mesh_boundary_ids.size() << std::endl;

      // Pack our boundary IDs into a vector for communication
      std::vector<BoundaryID> mesh_boundary_ids_vector(_mesh_boundary_ids.begin(),
                                                       _mesh_boundary_ids.end());

      // Gather them all into an enlarged vector
      Parallel::allgather(mesh_boundary_ids_vector);

      // Attempt to insert any new IDs into the set (any existing ones will be skipped)
      _mesh_boundary_ids.insert(mesh_boundary_ids_vector.begin(),
				mesh_boundary_ids_vector.end());

      // Boundary ID size after
      // std::cout << "(after) _mesh_boundary_ids.size()=" << _mesh_boundary_ids.size() << std::endl;
    }

  update();
}

void
MooseMesh::update()
{
  // Rebuild the boundary conditions
  build_node_list_from_side_list();

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
  if(i > _mesh.max_node_id())
    return *(*_quadrature_nodes.find(i)).second;

  return _mesh.node(i);
}

Node &
MooseMesh::node(const unsigned int i)
{
  if(i > _mesh.max_node_id())
    return *_quadrature_nodes[i];

  return _mesh.node(i);
}

const Node*
MooseMesh::node_ptr(const unsigned int i) const
{
  if(i > _mesh.max_node_id())
    return (*_quadrature_nodes.find(i)).second;

  return _mesh.node_ptr(i);
}

Node*
MooseMesh::node_ptr(const unsigned int i)
{
  if(i > _mesh.max_node_id())
    return _quadrature_nodes[i];

  return _mesh.node_ptr(i);
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
    Elem * elem = _mesh.elem(*it);
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

void
MooseMesh::buildNodeList()
{
  freeBndNodes();

  /// Boundary node list (node ids and corresponding side-set ids, arrays always have the same length)
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;
  _mesh.boundary_info->build_node_list(nodes, ids);

  int n = nodes.size();
  _bnd_nodes.resize(n);
  for (int i = 0; i < n; i++)
  {
    _bnd_nodes[i] = new BndNode(&_mesh.node(nodes[i]), ids[i]);
    _node_set_nodes[ids[i]].push_back(nodes[i]);
  }

  _bnd_nodes.reserve(_bnd_nodes.size() + _extra_bnd_nodes.size());
  for(unsigned int i=0; i<_extra_bnd_nodes.size(); i++)
  {
    BndNode * bnode = new BndNode(_extra_bnd_nodes[i]._node, _extra_bnd_nodes[i]._bnd_id);
    _bnd_nodes.push_back(bnode);
  }
}

void
MooseMesh::buildBndElemList()
{
  freeBndElems();

  /// Boundary node list (node ids and corresponding side-set ids, arrays always have the same length)
  std::vector<unsigned int> elems;
  std::vector<unsigned short int> sides;
  std::vector<boundary_id_type> ids;
  _mesh.boundary_info->build_side_list(elems, sides, ids);

  int n = elems.size();
  _bnd_elems.resize(n);
  for (int i = 0; i < n; i++)
  {
    _bnd_elems[i] = new BndElement(_mesh.elem(elems[i]), sides[i], ids[i]);
  }
}

std::map<unsigned int, std::vector<unsigned int> > &
MooseMesh::nodeToElemMap()
{
  if(!_node_to_elem_map_built)
  {
    _node_to_elem_map_built = true;
    MeshBase::const_element_iterator       el  = _mesh.elements_begin();
    const MeshBase::const_element_iterator end = _mesh.elements_end();

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
    _active_local_elem_range = new ConstElemRange(_mesh.active_local_elements_begin(),
                                                  _mesh.active_local_elements_end(), GRAIN_SIZE);
  }

  return _active_local_elem_range;
}

NodeRange *
MooseMesh::getActiveNodeRange()
{
  if (!_active_node_range)
  {
    _active_node_range = new NodeRange(_mesh.active_nodes_begin(),
                                       _mesh.active_nodes_end(), GRAIN_SIZE);
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
    _active_semilocal_node_range = new NodeRange(_mesh.local_nodes_begin(),
                                                 _mesh.local_nodes_end(), GRAIN_SIZE);
  }
 */

  return _active_semilocal_node_range;
}

ConstNodeRange *
MooseMesh::getLocalNodeRange()
{
  if (!_local_node_range)
  {
    _local_node_range = new ConstNodeRange(_mesh.local_nodes_begin(),
                                           _mesh.local_nodes_end(), GRAIN_SIZE);
  }

  return _local_node_range;
}

ConstBndNodeRange *
MooseMesh::getBoundaryNodeRange()
{
  if (!_bnd_node_range)
  {
    _bnd_node_range = new ConstBndNodeRange(bnd_nodes_begin(),
                                            bnd_nodes_end(), GRAIN_SIZE);
  }

  return _bnd_node_range;
}

ConstBndElemRange *
MooseMesh::getBoundaryElementRange()
{
  if (!_bnd_elem_range)
  {
    _bnd_elem_range = new ConstBndElemRange(bnd_elems_begin(),
                                            bnd_elems_end(), GRAIN_SIZE);
  }

  return _bnd_elem_range;
}

// MooseMesh Modifiers /////
void
MooseMesh::addMeshModifer(const std::string & mod_name, const std::string & name, InputParameters parameters)
{
  _mesh_modifiers.push_back(static_cast<MeshModifier *>(Factory::instance()->create(mod_name, name, parameters)));
}

void
MooseMesh::applyMeshModifications()
{
  for (std::vector<MeshModifier *>::iterator i = _mesh_modifiers.begin(); i != _mesh_modifiers.end(); ++i)
    (*i)->modifyMesh(_mesh);
}

void
MooseMesh::cacheInfo()
{
  for (MeshBase::element_iterator el = _mesh.elements_begin(); el != _mesh.elements_end(); ++el)
  {
    Elem * elem = *el;
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
MooseMesh::bnd_nodes_begin ()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.begin(), _bnd_nodes.end(), p);
}

// default end() accessor
bnd_node_iterator
MooseMesh::bnd_nodes_end ()
{
  Predicates::NotNull<bnd_node_iterator_imp> p;
  return bnd_node_iterator(_bnd_nodes.end(), _bnd_nodes.end(), p);
}

// default begin() accessor
bnd_elem_iterator
MooseMesh::bnd_elems_begin ()
{
  Predicates::NotNull<bnd_elem_iterator_imp> p;
  return bnd_elem_iterator(_bnd_elems.begin(), _bnd_elems.end(), p);
}

// default end() accessor
bnd_elem_iterator
MooseMesh::bnd_elems_end ()
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
  if (_mesh.n_nodes() != _node_map.size())
  {
    _node_map.clear();
    _node_map.reserve(_mesh.n_nodes());
    for (libMesh::MeshBase::node_iterator i=_mesh.nodes_begin(); i!=_mesh.nodes_end(); ++i)
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
    node = _mesh.add_node(new Node(p));
    _node_map.push_back(node);
  }

/*  Alternative method
  libMesh::MeshBase::node_iterator i = _mesh.nodes_begin();
  libMesh::MeshBase::node_iterator i_end = _mesh.nodes_end();
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
    node = _mesh.add_node(new Node(p));
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

    if(new_id <= _mesh.max_node_id())
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

  _extra_bnd_nodes.push_back(*bnode);

  // Do this so the range will be regenerated next time it is accessed
  delete _bnd_node_range;
  _bnd_node_range = NULL;

  return qnode;
}

Node *
MooseMesh::getQuadratureNode(const Elem * elem, const unsigned short int side, const unsigned int qp)
{
  return _elem_to_side_to_qp_to_quadrature_nodes[elem->id()][side][qp];
}

BoundaryID
MooseMesh::getBoundaryID(const BoundaryName & boundary_name) const
{
  BoundaryID id;
  std::istringstream ss(boundary_name);

  if (!(ss >> id))
    id = _mesh.boundary_info->get_id_by_name(boundary_name);

  return id;
}

std::vector<BoundaryID>
MooseMesh::getBoundaryIDs(const std::vector<BoundaryName> & boundary_name) const
{
  std::vector<BoundaryID> ids(boundary_name.size());

  for(unsigned int i=0; i<boundary_name.size(); i++)
  {
    BoundaryID id;
    std::istringstream ss(boundary_name[i]);

    if (!(ss >> id))
      id = _mesh.boundary_info->get_id_by_name(boundary_name[i]);

    ids[i] = id;
  }

  return ids;
}

SubdomainID
MooseMesh::getSubdomainID(const SubdomainName & subdomain_name) const
{
  SubdomainID id;
  std::istringstream ss(subdomain_name);

  if (!(ss >> id))
    id = _mesh.get_id_by_name(subdomain_name);

  return id;
}

std::vector<SubdomainID>
MooseMesh::getSubdomainIDs(const std::vector<SubdomainName> & subdomain_name) const
{
  std::vector<SubdomainID> ids(subdomain_name.size());

  for(unsigned int i=0; i<subdomain_name.size(); i++)
  {
    SubdomainID id;
    std::istringstream ss(subdomain_name[i]);

    if (!(ss >> id))
      id = _mesh.get_id_by_name(subdomain_name[i]);

    ids[i] = id;
  }

  return ids;
}

void
MooseMesh::setSubdomainName(SubdomainID subdomain_id, SubdomainName name)
{
  _mesh.subdomain_name(subdomain_id) = name;
}

void
MooseMesh::setBoundaryName(BoundaryID boundary_id, BoundaryName name)
{
  std::vector<BoundaryID> side_boundaries;
  _mesh.boundary_info->build_side_boundary_ids(side_boundaries);

  // We need to figure out if this boundary is a sideset or nodeset
  if (std::find(side_boundaries.begin(), side_boundaries.end(), boundary_id) != side_boundaries.end())
    _mesh.boundary_info->sideset_name(boundary_id) = name;
  else
    _mesh.boundary_info->nodeset_name(boundary_id) = name;
}

void
MooseMesh::buildPeriodicNodeMap(std::multimap<unsigned int, unsigned int> & periodic_node_map, unsigned int var_number, PeriodicBoundaries *pbs) const
{
  periodic_node_map.clear();

  MeshBase::const_element_iterator it = _mesh.active_local_elements_begin();
  MeshBase::const_element_iterator it_end = _mesh.active_local_elements_end();
  const PointLocatorBase &point_locator = _mesh.point_locator();

  for (; it != it_end; ++it)
  {
    const Elem *elem = *it;
    for (unsigned int s=0; s<elem->n_sides(); ++s)
    {
      if (elem->neighbor(s))
        continue;

      const std::vector<boundary_id_type>& bc_ids = _mesh.boundary_info->boundary_ids (elem, s);
      for (std::vector<boundary_id_type>::const_iterator id_it = bc_ids.begin(); id_it!=bc_ids.end(); ++id_it)
      {
        const boundary_id_type boundary_id = *id_it;
        const PeriodicBoundaryBase *periodic = pbs->boundary(boundary_id);
        if (periodic && periodic->is_my_variable(var_number))
        {
          const Elem* neigh = pbs->neighbor(boundary_id, point_locator, elem, s);
          unsigned int s_neigh = _mesh.boundary_info->side_with_boundary_id (neigh, periodic->pairedboundary);

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

  _mesh.boundary_info->build_node_list(nl, il);

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

