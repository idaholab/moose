//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainExtraElementIDGenerator.h"

registerMooseObject("ReactorApp", SubdomainExtraElementIDGenerator);

#include "MooseMeshUtils.h"

#include "libmesh/elem.h"

InputParameters
SubdomainExtraElementIDGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>(
      "input", "Name of an existing mesh generator to which we assign element IDs");
  params.addRequiredParam<std::vector<SubdomainName>>("subdomains",
                                                      "Subdomain names present in the input mesh");
  params.addRequiredParam<std::vector<std::string>>("extra_element_id_names",
                                                    "List of user-defined extra element ID names");
  params.addRequiredParam<std::vector<std::vector<dof_id_type>>>(
      "extra_element_ids",
      "User-defined extra element IDs corresponding to 'subdomains' in the same order");

  params.addParam<std::vector<dof_id_type>>(
      "default_extra_element_ids", "Default extra element IDs for elements not in 'subdomains'");

  params.addClassDescription(
      "Assign extra element IDs for elements on a mesh based on mesh subdomains.");
  return params;
}

SubdomainExtraElementIDGenerator::SubdomainExtraElementIDGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _subdomain_names(getParam<std::vector<SubdomainName>>("subdomains"))
{
  if (_subdomain_names.size() == 0)
    paramError("subdomains", "Empty subdomain vector provided!");
}

std::unique_ptr<MeshBase>
SubdomainExtraElementIDGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // construct a map from the subdomain ID to the index in 'subdomains'
  auto subdomain_ids = MooseMeshUtils::getSubdomainIDs(*mesh, _subdomain_names);

  // check that all subdomains are present
  for (const auto & name : _subdomain_names)
    if (!MooseMeshUtils::hasSubdomainName(*mesh, name))
      paramError("subdomains", "Subdomain " + name + " does not exist in the mesh");

  // check to make sure no duplicated subdomain ids
  std::set<SubdomainID> unique_subdomain_ids;
  for (const auto & id : subdomain_ids)
    if (unique_subdomain_ids.count(id) > 0)
      paramError("subdomains", "Cannot have subdomain with ID ", id, " listed more than once!");
    else
      unique_subdomain_ids.insert(id);

  std::map<SubdomainID, unsigned int> subdomains;
  for (unsigned int i = 0; i < _subdomain_names.size(); ++i)
    subdomains[subdomain_ids[i]] = i;

  auto & element_id_names = getParam<std::vector<std::string>>("extra_element_id_names");
  auto & element_ids = getParam<std::vector<std::vector<dof_id_type>>>("extra_element_ids");

  if (element_id_names.size() != element_ids.size())
    paramError("extra_element_ids", "Inconsistent vector size for element IDs");
  for (auto & element_id : element_ids)
  {
    if (_subdomain_names.size() != element_id.size())
      paramError("extra_element_ids", "Inconsistent vector size for element IDs");
  }

  // get indices for all extra element integers
  std::vector<unsigned int> extra_id_indices;
  for (auto & element_id_name : element_id_names)
  {
    if (!mesh->has_elem_integer(element_id_name))
      extra_id_indices.push_back(mesh->add_elem_integer(element_id_name));
    else
      extra_id_indices.push_back(mesh->get_elem_integer_index(element_id_name));
  }

  if (isParamValid("default_extra_element_ids"))
  {
    auto & default_ids = getParam<std::vector<dof_id_type>>("default_extra_element_ids");
    if (default_ids.size() != element_id_names.size())
      paramError("default_extra_element_ids", "Inconsistent vector size for default element IDs");

    for (auto & elem : mesh->element_ptr_range())
      for (unsigned int i = 0; i < element_ids.size(); ++i)
        elem->set_extra_integer(extra_id_indices[i], default_ids[i]);
  }

  for (auto & elem : mesh->element_ptr_range())
  {
    SubdomainID id = elem->subdomain_id();
    auto it = subdomains.find(id);
    if (it == subdomains.end())
      continue;

    for (unsigned int i = 0; i < element_ids.size(); ++i)
      elem->set_extra_integer(extra_id_indices[i], element_ids[i][it->second]);
  }

  return mesh;
}
