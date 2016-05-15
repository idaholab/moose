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

#include "ReassignSubdomainIDs.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ReassignSubdomainIDs>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<std::vector<SubdomainID> >("block", "Current subdomain IDs");
  params.addRequiredParam<std::vector<SubdomainID> >("new_block", "New subdomain IDs");
  return params;
}

ReassignSubdomainIDs::ReassignSubdomainIDs(const InputParameters & parameters) :
    MeshModifier(parameters),
    _block(getParam<std::vector<SubdomainID> >("block")),
    _new_block(getParam<std::vector<SubdomainID> >("new_block"))
{
  if (_block.size() != _new_block.size())
    mooseError("`block` and `new_block` MUST have the same length for ReassignSubdomainIDs " << _name);

  // Create the subdomain ID map
  for (unsigned int i=0; i < _block.size(); i++)
    _block_map[_block[i]] = _new_block[i];
}

ReassignSubdomainIDs::~ReassignSubdomainIDs()
{
}

void
ReassignSubdomainIDs::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling ReassignSubdomainIDs::modify()");

  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  ElemRange elem_range(mesh.elements_begin(), mesh.elements_end(), 1);

  Threads::parallel_for(elem_range,
                        [&](const ElemRange & elem_range)
                        {
                          // Save this off
                          auto block_map_end = _block_map.end();

                          for (auto & elem : elem_range)
                          {
                            auto & sid = elem->subdomain_id();

                            auto entry = _block_map.find(sid);

                            if (entry != block_map_end)
                              sid = entry->second;
                          }
                        });
}
