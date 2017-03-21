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

#include "AssignElementSubdomainID.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<AssignElementSubdomainID>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<std::vector<SubdomainID>>("subdomain_ids",
                                                    "New subdomain IDs of all elements");
  params.addParam<std::vector<dof_id_type>>("element_ids", "New subdomain IDs of all elements");
  return params;
}

AssignElementSubdomainID::AssignElementSubdomainID(const InputParameters & parameters)
  : MeshModifier(parameters)
{
}

void
AssignElementSubdomainID::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SubdomainBoundingBox::modify()");

  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  std::vector<SubdomainID> bids = getParam<std::vector<SubdomainID>>("subdomain_ids");

  // Generate a list of elements to which new subdomain IDs are to be assigned
  std::vector<Elem *> elements;
  if (isParamValid("element_ids"))
  {
    std::vector<dof_id_type> elemids = getParam<std::vector<dof_id_type>>("element_ids");
    for (const auto & dof : elemids)
    {
      Elem * elem = mesh.query_elem_ptr(dof);
      if (!elem)
        mooseError("invalid element ID is in element_ids");
      else
        elements.push_back(elem);
    }
  }
  else
  {
    bool has_warned_remapping = false;
    MeshBase::const_element_iterator el = mesh.elements_begin();
    const MeshBase::const_element_iterator end_el = mesh.elements_end();
    for (dof_id_type e = 0; el != end_el; ++el, ++e)
    {
      Elem * elem = *el;
      if (elem->id() != e && (!has_warned_remapping))
      {
        mooseWarning("AssignElementSubdomainID will ignore the element remapping");
        has_warned_remapping = true;
      }
      elements.push_back(elem);
    }
  }

  if (bids.size() != elements.size())
    mooseError(" Size of subdomain_ids is not consistent with the number of elements");

  // Assign new subdomain IDs and make sure elements in different types are not assigned with the
  // same subdomain ID
  std::map<ElemType, std::set<SubdomainID>> type2blocks;
  for (dof_id_type e = 0; e < elements.size(); ++e)
  {
    Elem * elem = elements[e];
    ElemType type = elem->type();
    SubdomainID newid = bids[e];

    bool has_type = false;
    for (auto & it : type2blocks)
    {
      if (it.first == type)
      {
        has_type = true;
        it.second.insert(newid);
      }
      else if (it.second.count(newid) > 0)
        mooseError("trying to assign elements with different types with the same subdomain ID");
    }

    if (!has_type)
    {
      std::set<SubdomainID> blocks;
      blocks.insert(newid);
      type2blocks.insert(std::make_pair(type, blocks));
    }

    elem->subdomain_id() = newid;
  }
}
