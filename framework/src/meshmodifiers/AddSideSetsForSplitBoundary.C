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

#include "AddSideSetsForSplitBoundary.h"

#include "MooseMesh.h"
#include "MooseUtils.h"

template<>
InputParameters validParams<AddSideSetsForSplitBoundary>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<std::vector<std::vector<SubdomainName> > >("subdomain_blocks", "Multiple semicolon separated groups of space separated block names");
  params.addClassDescription("Add separated side sets to boundaries which are intersected by subdomain interface in multiscale calculations.");
  params.addParam<std::vector<SubdomainName> >("subdomain_names", "Names of physical subdomains");
  return params;
}

AddSideSetsForSplitBoundary::AddSideSetsForSplitBoundary(const InputParameters & parameters) :
    MeshModifier(parameters)
{
  std::vector<std::vector<SubdomainName> > subdomains = getParam<std::vector<std::vector<SubdomainName> > >("subdomain_blocks");
  _num_subdomains = subdomains.size();

  unsigned int nblocks = 0;
  std::set<SubdomainName> check_name;
  _num_subdomain_blocks.resize(_num_subdomains);
  _subdomain_blocks.resize(_num_subdomains);

  for (unsigned int i = 0; i < subdomains.size(); ++i)
  {
    _num_subdomain_blocks[i] = subdomains[i].size();
    nblocks += _num_subdomain_blocks[i];

    for (unsigned int j = 0; j < _num_subdomain_blocks[i]; ++j)
    {
      check_name.insert(subdomains[i][j]);
      SubdomainID id = _mesh_ptr->getSubdomainID(subdomains[i][j]);
      _subdomain_blocks[i].insert(id);
    }
  }

  if (check_name.size() != nblocks)
    mooseError("block names in subdomain_blocks are not unique");

  if (isParamValid("subdomain_names"))
    _subdomain_names = getParam<std::vector<SubdomainName> >("subdomain_names");
  else
  {
    std::stringstream ss;
    if (_num_subdomains <= 10)
      ss << std::setw(1);
    else if (_num_subdomains <= 100)
      ss << std::setw(2) << std::setfill('0');
    else if (_num_subdomains <= 1000)
      ss << std::setw(3) << std::setfill('0');
    else
      ss << std::setw(6) << std::setfill('0');

    for (unsigned int i = 0; i < _num_subdomains; ++i)
    {
      ss << "subdomain-" << i;
      _subdomain_names.push_back(ss.str());
      ss.str("");
    }
  }
}

AddSideSetsForSplitBoundary::~AddSideSetsForSplitBoundary()
{
}

void
AddSideSetsForSplitBoundary::modify()
{
  // get the mesh
  MeshBase & mesh = _mesh_ptr->getMesh();

  std::set<SubdomainID> all_mesh_blocks = _mesh_ptr->meshSubdomains();
  const MeshBase::element_iterator el_end = mesh.elements_end();
  for (MeshBase::element_iterator el = mesh.elements_begin(); el != el_end; ++el)
    all_mesh_blocks.insert((*el)->subdomain_id());
  if (!mesh.is_serial())
    _communicator.set_union(all_mesh_blocks);

  unsigned int nblocks = 0;
  for (unsigned int i = 0; i < _num_subdomains; ++i)
    nblocks += _num_subdomain_blocks[i];
  if (all_mesh_blocks.size() != nblocks)
  {
    mooseWarning("Not all blocks are covered by input subdomains.");
    std::set<SubdomainID> all_blocks;
    for (unsigned int i = 0; i < _num_subdomains; ++i)
      all_blocks.insert(_subdomain_blocks[i].begin(), _subdomain_blocks[i].end());

    std::set<SubdomainID> excess_blocks;
    std::set_difference(all_mesh_blocks.begin(), all_mesh_blocks.end(), all_blocks.begin(), all_blocks.end(), std::inserter(excess_blocks, excess_blocks.end()));

    if (excess_blocks.size() == 0)
      mooseError("excess_blocks is empty but it should not be.");

    _num_subdomains++;
    _num_subdomain_blocks.push_back(excess_blocks.size());
    _subdomain_blocks.push_back(excess_blocks);

    std::stringstream ss;
    ss << "subdomain-" << _num_subdomains-1;
    _subdomain_names.push_back(ss.str());
  }

  // create an array to store subdomain IDs for all elements
  dof_id_type maxid = mesh.max_elem_id();
  // invalid subdomain ID for initialization
  std::vector<unsigned int> subdomain_ids(maxid, _num_subdomains);

  // get all subdomain IDs for all elements
  MeshBase::const_element_iterator           el = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.active_elements_end();
  for (; el != end_el; ++el)
  {
    const Elem* elem = *el;
    for (unsigned int i = 0; i < _num_subdomains; ++i)
      if (_subdomain_blocks[i].count(elem->subdomain_id())>0) subdomain_ids[elem->id()] = i;
  }

  // get a copy of boundary_info before we add side sets
  BoundaryInfo boundary_info_old = mesh.get_boundary_info();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // get all side set names
  const std::set<BoundaryID> & existing_boundary_ids = _mesh_ptr->getBoundaryIDs();
  std::vector<BoundaryName> boundary_names;
  std::map<BoundaryID, unsigned int> boundary_map;

  for (std::set<BoundaryID>::const_iterator it = existing_boundary_ids.begin(); it != existing_boundary_ids.end(); ++it)
  {
    boundary_map[*it] = boundary_names.size();
    std::string bname = boundary_info.get_sideset_name(*it);
    // if no name specified to a boundary, give it a name
    if (bname == "")
    {
      std::stringstream ss;
      ss << *it;
      bname = ss.str();
    }
    // give names to boundary side sets based on boundary names and subdomain names where the side belongs
    for (unsigned int i = 0; i < _num_subdomains; ++i)
      boundary_names.push_back(bname + "_" + _subdomain_names[i] + "_bnd");
  }

  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);
  std::vector<unsigned int> bnd_counter(boundary_ids.size());

  // the following quantities are used to check if boundary side sets are unique to boundary sides
  unsigned int uncovered_bnd_sides = 0;
  unsigned int overcovered_bnd_sides = 0;

  // add boundary side sets
  for (el = mesh.active_elements_begin(); el != end_el; ++el)
  {
    Elem * elem = *el;
    unsigned int curr_subdomain = subdomain_ids[elem->id()];
    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      if (!elem->neighbor(side))
      {
        // inquire boundary IDs only for domain boundary sides
        std::vector<boundary_id_type> side_boundary_ids = boundary_info_old.boundary_ids(elem, side);

        // check for boundary sides with zero or multiple boundary IDs
        if (side_boundary_ids.size() == 0)
          uncovered_bnd_sides++;

        if (side_boundary_ids.size() > 1)
          overcovered_bnd_sides++;

        // add the domain boundary side set
        for (unsigned int i = 0; i < side_boundary_ids.size(); ++i)
        {
          for (unsigned int j = 0; j < _num_subdomains; ++j)
          {
            if (j == curr_subdomain)
            {
              boundary_info.add_side(elem, side, boundary_ids[boundary_map[side_boundary_ids[i]] + j]);
              // bnd_counter will make sure only domain boundary will have a nonzero counter
              bnd_counter[boundary_map[side_boundary_ids[i]] + j]++;
            }
          }
        }
      }
    }
  }

  if (uncovered_bnd_sides > 0)
  {
    std::stringstream ss;
    ss << uncovered_bnd_sides;
    mooseWarning(ss.str() << " boundary sides not belonging to any side sets are detected.");
  }
  if (overcovered_bnd_sides > 0)
  {
    std::stringstream ss;
    ss << overcovered_bnd_sides;
    mooseWarning(ss.str() + "boundary sides are covered by more than one side set.");
  }

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
  {
    // only give names to non-interface sides, so it does not matter whether or not to use the modifier before SideSetsBetweenSubdomains. The rationale behind this is bnd_counter will be nonzero only for boundary sides
    if (bnd_counter[i]>0)
      boundary_info.sideset_name(boundary_ids[i]) = boundary_names[i];
  }

  mesh.prepare_for_use();
}
