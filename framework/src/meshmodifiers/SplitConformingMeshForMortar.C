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

#include "SplitConformingMeshForMortar.h"

template<>
InputParameters validParams<SplitConformingMeshForMortar>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<unsigned int>("num_subdomains", "Number of subdomains for mortar FEM");
  params.addRequiredParam<std::vector<unsigned int> >("num_subdomain_blocks", "Number of blocks on all subdomains");
  params.addRequiredParam<std::vector<SubdomainName> >("subdomain_blocks", "The block names of all blocks of all subdomains");
  return params;
}

SplitConformingMeshForMortar::SplitConformingMeshForMortar(const std::string & name, InputParameters parameters):
    MeshModifier(name, parameters),
    _num_subdomains(getParam<unsigned int>("num_subdomains")),
    _num_subdomain_blocks(getParam<std::vector<unsigned int> >("num_subdomain_blocks"))
{
  if (_num_subdomain_blocks.size() != _num_subdomains) mooseError("num_subdomain_blocks is not in size of num_subdomains");

  unsigned int nblocks = 0;
  for (unsigned int i=0; i<_num_subdomains; i++)
    nblocks += _num_subdomain_blocks[i];

  std::vector<SubdomainName> blocks = getParam<std::vector<SubdomainName> >("subdomain_blocks");
  if (blocks.size() != nblocks) mooseError("subdomain_blocks is not in size of total number of blocks of all subdomains");

  std::set<SubdomainName> check_name;
  for (unsigned int i=0; i<nblocks; i++)
    check_name.insert(blocks[i]);
  if (check_name.size() != nblocks) mooseError("block names in subdoman_blocks are not unique");

  _subdomain_blocks.resize(_num_subdomains);
  unsigned int nblk = _num_subdomain_blocks[0];
  unsigned int subdomain_id = 0;
  for (unsigned int i=0; i<nblocks; i++)
  {
    SubdomainID id = _mesh_ptr->getSubdomainID(blocks[i]);
    if (i>=nblk)
    {
      subdomain_id++;
      nblk += _num_subdomain_blocks[subdomain_id];
    }
    _subdomain_blocks[subdomain_id].insert(id);
  }
  /*
  for (unsigned int i=0; i<_subdomain_blocks.size(); i++)
  {
    Moose::out << std::endl;
    for (std::set<SubdomainID>::iterator it = _subdomain_blocks[i].begin(); it!=_subdomain_blocks[i].end(); it++)
      Moose::out << " " << *it;
    Moose::out << std::endl;
  }
  */
}

SplitConformingMeshForMortar::~SplitConformingMeshForMortar()
{
}

void
SplitConformingMeshForMortar::modify()
{
  // get the reference of the wrapped libMesh mesh
  MeshBase & mesh = _mesh_ptr->getMesh();

  // create a array for storing the subdomain IDs of all elements
  // Note: here subdomain is not the one in libMesh, we will refer subdomain in libMesh as block later.
  dof_id_type maxid = mesh.max_elem_id();
  unsigned int * subdomain_ids = new unsigned int[maxid];

  // loop through all elements to get their subdomain ids
  MeshBase::const_element_iterator          el  = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.active_elements_end();
  for (; el != end_el ; ++el)
  {
    const Elem* elem = *el;
    SubdomainID curr_block = elem->subdomain_id();

    // default to a invalid subdomain ID
    subdomain_ids[elem->id()] = _num_subdomains;
    for (unsigned int i=0; i<_num_subdomains; i++)
      if (_subdomain_blocks[i].count(curr_block) > 0) subdomain_ids[elem->id()] = i;
  }

  const subdomain_id_type INVALID_SUBDOMAIN_ID = std::numeric_limits<subdomain_id_type>::max();
  std::vector<std::vector<subdomain_id_type> > connectivity(_num_subdomains);
  for (unsigned int i=0; i<_num_subdomains; i++)
    connectivity[i].resize(_num_subdomains, INVALID_SUBDOMAIN_ID);
  subdomain_id_type block_counter = mesh.n_subdomains();
  dof_id_type elem_counter = mesh.max_elem_id();

  // create a array for storing the node IDs used by all the subdomainsr
  dof_id_type maxnodeid = mesh.max_node_id();
  dof_id_type * node_id_on_subdomains = new dof_id_type[maxnodeid * _num_subdomains];
  for (dof_id_type i=0; i<maxnodeid*_num_subdomains; i++)
    node_id_on_subdomains[i] = DofObject::invalid_id;
  dof_id_type node_counter = maxnodeid;

  std::vector<Elem *> faces;

  // loop through all elements again to create the mortar faces and nodes for splitting the mesh
  for (el = mesh.active_elements_begin(); el != end_el ; ++el)
  {
    Elem* elem = *el;
    unsigned int curr_subdomain = subdomain_ids[elem->id()];

    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      Elem * neighbor = elem->neighbor(side);
      if (neighbor)
      {
        // we only need to visit the pair once
        if (neighbor->id() < elem->id()) continue;

        unsigned int neig_subdomain = subdomain_ids[neighbor->id()];
        if (neig_subdomain != curr_subdomain)
        {
          // record the interface when first meet
          if (connectivity[curr_subdomain][neig_subdomain] == INVALID_SUBDOMAIN_ID)
          {
            connectivity[curr_subdomain][neig_subdomain] = block_counter;
            connectivity[neig_subdomain][curr_subdomain] = block_counter;
            std::stringstream ss;
            ss << "interface_" << curr_subdomain << "_" << neig_subdomain;
            mesh.subdomain_name(block_counter) = ss.str();
            ss.str("");
            block_counter++;
          }
          subdomain_id_type interface_id = connectivity[curr_subdomain][neig_subdomain];

          // build face element and added into mesh for mortar faces
          // Note: we need to use proxy=false to indeed create the face element, which has its own connectivity to nodes.
          Elem * face = elem->build_side(side, false).release();
          face->set_id() = elem_counter;
          face->subdomain_id() = interface_id;
          elem_counter++;
          faces.push_back(face);

          // now we need to duplicate all nodes on the face to break the connectivity
          for (unsigned int node=0; node<face->n_nodes(); node++)
          {
            dof_id_type nodeid = face->node(node);

            dof_id_type dup_nodeid = node_id_on_subdomains[curr_subdomain*maxnodeid + nodeid];
            if (dup_nodeid == DofObject::invalid_id)
            {
              // duplicate the node and get its id
              Node * p = (new Node(*(face->get_node(node))));
              p->set_id() = node_counter++;
              // add the node into the mesh
              mesh.add_node(p);
              node_id_on_subdomains[curr_subdomain*maxnodeid + nodeid] = p->id();
            }

            dup_nodeid = node_id_on_subdomains[neig_subdomain*maxnodeid + nodeid];
            if (dup_nodeid == DofObject::invalid_id)
            {
              // duplicate the node and get its id
              Node * p = (new Node(*(face->get_node(node))));
              p->set_id() = node_counter++;
              // add the node into the mesh
              mesh.add_node(p);
              node_id_on_subdomains[neig_subdomain*maxnodeid + nodeid] = p->id();
            }
          }
        }
      }
    }
  }

  /*
  Moose::out << "node counter=" << node_counter << std::endl;
  for (unsigned int i=0; i<faces.size(); i++)
  {
    for (unsigned int node=0; node<faces[i]->n_nodes(); node++)
      Moose::out << " " << faces[i]->node(node);
    Moose::out << std::endl;
  }

  Moose::out << "new node for subdomains: " << std::endl;
  for (unsigned int j=0; j<_num_subdomains; j++)
  {
    for (unsigned int i=0; i<maxnodeid; i++)
      Moose::out << " " << node_id_on_subdomains[j*maxnodeid + i];
    Moose::out << std::endl;
  }
  */

  // loop through all elements again to actually split the mesh by resetting the connectivity
  dof_id_type face_counter = 0;
  for (el = mesh.active_elements_begin(); el != end_el ; ++el)
  {
    Elem* elem = *el;
    unsigned int curr_subdomain = subdomain_ids[elem->id()];

    for (unsigned int node=0; node<elem->n_nodes(); node++)
    {
      dof_id_type nodeid = elem->node(node);
      if (node_id_on_subdomains[curr_subdomain*maxnodeid + nodeid] != DofObject::invalid_id)
      {
        Node * p = mesh.node_ptr(node_id_on_subdomains[curr_subdomain*maxnodeid + nodeid]);
        elem->set_node(node) = p;
      }
    }
  }

  delete node_id_on_subdomains;

  // Get a copy to our BoundaryInfo object for later use
  // FIXME: not sure if we need to make a copy of boundary info here
  BoundaryInfo boundary_info_old = mesh.get_boundary_info();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // get all side sets and prepare to split them to create new side sets
  const std::set<BoundaryID> & existing_boundary_ids = _mesh_ptr->getBoundaryIDs();
  std::vector<BoundaryName> boundary_names;
  std::map<BoundaryID, unsigned int> boundary_map;
  for (std::set<BoundaryID>::const_iterator it = existing_boundary_ids.begin();
       it != existing_boundary_ids.end(); it++)
  {
    boundary_map[*it] = boundary_names.size();
    std::string bname = boundary_info.get_sideset_name(*it);
    if (bname == "")
    {
      std::stringstream ss;
      ss << *it;
      bname = ss.str();
      ss.str("");
    }
    for (unsigned int i=0; i<_num_subdomains; i++)
    {
      std::stringstream ss;
      ss << "subdomain-" << i;
      boundary_names.push_back(bname+"-"+ss.str()+"-interior");
      boundary_names.push_back(bname+"_"+ss.str()+"-boundary");
      boundary_names.push_back(bname+"_"+ss.str()+"-outside");
      ss.str("");
    }
  }

  // new side sets for subdomain interfaces
  std::vector<std::vector<unsigned int> > interface_ids(_num_subdomains);
  for (unsigned int i=0; i<_num_subdomains; i++)
    interface_ids[i].resize(_num_subdomains);
  for (unsigned int i=0; i<_num_subdomains; i++)
    for (unsigned int j=0; j<_num_subdomains; j++)
    {
      if (j==i) continue;
      if (connectivity[i][j] != INVALID_SUBDOMAIN_ID)
      {
        interface_ids[i][j] = boundary_names.size();
        std::stringstream ss;
        ss << "interface-" << i << "-to-" << j;
        boundary_names.push_back(ss.str());
      }
    }
  connectivity.clear();

  // assign boundary IDs of these new side sets
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);

  // loop through all elements again to add sides into side sets
  for (el = mesh.active_elements_begin(); el != end_el ; ++el)
  {
    Elem* elem = *el;
    unsigned int curr_subdomain = subdomain_ids[elem->id()];

    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      std::vector<boundary_id_type> side_boundary_ids = boundary_info_old.boundary_ids(elem, side);

      Elem * neighbor = elem->neighbor(side);

      if (neighbor)
      {
        unsigned int neig_subdomain = subdomain_ids[neighbor->id()];
        if (neig_subdomain != curr_subdomain)
        {
          for (unsigned int i=0; i<side_boundary_ids.size(); i++)
            for (unsigned int j=0; j<_num_subdomains; j++)
            {
              if (j==curr_subdomain)
                boundary_info.add_side(elem, side, boundary_ids[boundary_map[side_boundary_ids[i]] + j*3 + 1]);
              else if (j==neig_subdomain)
              {
                //neighboring element will set this
              }
              else
              {
                if (neighbor->id()>elem->id())
                  boundary_info.add_side(elem, side, boundary_ids[boundary_map[side_boundary_ids[i]] + j*3 + 2]);
              }
            }

          unsigned int interface_id = interface_ids[curr_subdomain][neig_subdomain];
          boundary_info.add_side(elem, side, boundary_ids[interface_id]);
        }
        else
        {
          for (unsigned int i=0; i<side_boundary_ids.size(); i++)
            for (unsigned int j=0; j<_num_subdomains; j++)
            {
              if (j==curr_subdomain)
                boundary_info.add_side(elem, side, boundary_ids[boundary_map[side_boundary_ids[i]] + j*3]);
              else
                boundary_info.add_side(elem, side, boundary_ids[boundary_map[side_boundary_ids[i]] + j*3 + 2]);
            }
        }
      }
      else
      {
        for (unsigned int i=0; i<side_boundary_ids.size(); i++)
        {
          for (unsigned int j=0; j<_num_subdomains; j++)
          {
            if (j==curr_subdomain)
              boundary_info.add_side(elem, side, boundary_ids[boundary_map[side_boundary_ids[i]] + j*3 + 1]);
            else
              boundary_info.add_side(elem, side, boundary_ids[boundary_map[side_boundary_ids[i]] + j*3 + 2]);
          }
        }
      }
    }
  }

  delete subdomain_ids;

  for (unsigned int i=0; i<boundary_ids.size(); ++i)
    boundary_info.sideset_name(boundary_ids[i]) = boundary_names[i];

  // add the mortar elements
  for (unsigned int i=0; i<faces.size(); i++)
    mesh.add_elem(faces[i]);
}
