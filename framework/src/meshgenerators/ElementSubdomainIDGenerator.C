//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSubdomainIDGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", ElementSubdomainIDGenerator);

InputParameters
ElementSubdomainIDGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<SubdomainID>>("subdomain_ids",
                                                    "New subdomain IDs of all elements");
  params.addParam<std::vector<dof_id_type>>("element_ids", "New element IDs of all elements");
  params.addClassDescription(
      "Allows the user to assign each element the subdomain ID of their choice");
  return params;
}

ElementSubdomainIDGenerator::ElementSubdomainIDGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
ElementSubdomainIDGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  std::vector<SubdomainID> bids = getParam<std::vector<SubdomainID>>("subdomain_ids");

  // Generate a list of elements to which new subdomain IDs are to be assigned
  std::vector<Elem *> elements;
  if (isParamValid("element_ids"))
  {
    std::vector<dof_id_type> elemids = getParam<std::vector<dof_id_type>>("element_ids");
    for (const auto & dof : elemids)
    {
      Elem * elem = mesh->query_elem_ptr(dof);
      bool has_elem = elem;

      // If no processor sees this element, something must be wrong
      // with the specified ID.  If another processor sees this
      // element but we don't, we'll insert NULL into our elements
      // vector anyway so as to keep the indexing matching our bids
      // vector.
      this->comm().max(has_elem);
      if (!has_elem)
        mooseError("invalid element ID is in element_ids");
      else
        elements.push_back(elem);
    }
  }

  else
  {
    bool has_warned_remapping = false;

    // On a distributed mesh, iterating over all elements in
    // increasing order is tricky.  We have to consider element ids
    // which aren't on a particular processor because they're remote,
    // *and* elements which aren't on a particular processor because
    // there's a hole in the current numbering.
    //
    // I don't see how to do this without a ton of communication,
    // which is hopefully okay because it only happens at mesh setup,
    // and because nobody who is here trying to use
    // AssignElementSubdomainID to hand write every single element's
    // subdomain ID will have a huge number of elements on their
    // initial mesh.

    // Using plain max_elem_id() currently gives the same result on
    // every processor, but that isn't guaranteed by the libMesh
    // documentation, so let's be paranoid.
    dof_id_type end_id = mesh->max_elem_id();
    this->comm().max(end_id);

    for (dof_id_type e = 0; e != end_id; ++e)
    {
      // This is O(1) on ReplicatedMesh but O(log(N_elem)) on
      // DistributedMesh.  We can switch to more complicated but
      // asymptotically faster code if my "nobody who is here ... will
      // have a huge number of elements" claim turns out to be false.
      Elem * elem = mesh->query_elem_ptr(e);
      bool someone_has_elem = elem;
      if (!mesh->is_replicated())
        this->comm().max(someone_has_elem);

      if (elem && elem->id() != e && (!has_warned_remapping))
      {
        mooseWarning("AssignElementSubdomainID will ignore the element remapping");
        has_warned_remapping = true;
      }

      if (someone_has_elem)
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
    // Get the element we need to assign, or skip it if we just have a
    // nullptr placeholder indicating a remote element.
    Elem * elem = elements[e];
    if (!elem)
      continue;

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

  return dynamic_pointer_cast<MeshBase>(mesh);
}
