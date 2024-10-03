//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DepletionIDGenerator.h"
#include "MooseMeshUtils.h"
#include "libmesh/elem.h"

registerMooseObject("ReactorApp", DepletionIDGenerator);

InputParameters
DepletionIDGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>(
      "input", "Name of an existing mesh generator to which we assign element IDs");
  params.addParam<std::vector<ExtraElementIDName>>("id_name", "Extra integer id names");
  params.addParam<ExtraElementIDName>("material_id_name", "material_id", "Material id name");
  params.addParam<std::vector<dof_id_type>>(
      "exclude_material_id",
      "Define a list of material IDs to be excluded in the depletion ID generation");
  params.addParam<std::vector<ExtraElementIDName>>(
      "exclude_id_name", "Extra ID names that need to be excluded in the depletion ID generation");
  params.addParam<std::vector<std::vector<dof_id_type>>>(
      "exclude_id_value", "Extra ID values corresponding to names defined in `exclude_id_name`");
  params.addClassDescription("This DepletionIDGenerator source code is to assign depletion IDs for "
                             "elements on a mesh based on material and other extra element IDs.");
  return params;
}
DepletionIDGenerator::DepletionIDGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _material_id_name(getParam<ExtraElementIDName>("material_id_name"))
{
}

std::unique_ptr<MeshBase>
DepletionIDGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  // parsing the extra ids
  std::vector<ExtraElementIDName> id_names;
  id_names = getParam<std::vector<ExtraElementIDName>>("id_name");
  if (!mesh->has_elem_integer(_material_id_name))
    paramError("material_id_name",
               "Material ID name '",
               _material_id_name,
               "'is not defined in input mesh!");
  id_names.push_back(_material_id_name);
  auto parsed_ids = MooseMeshUtils::getExtraIDUniqueCombinationMap(*mesh, {}, id_names);
  // re-numbering if exclude_id_name is used
  if (isParamValid("exclude_id_name") && isParamValid("exclude_id_value"))
  {
    const auto exclude_id_name = getParam<std::vector<ExtraElementIDName>>("exclude_id_name");
    const auto & exclude_id_value =
        getParam<std::vector<std::vector<dof_id_type>>>("exclude_id_value");
    std::vector<unsigned int> id_index;
    std::vector<std::set<dof_id_type>> id_value_set;
    id_index.resize(exclude_id_name.size());
    id_value_set.resize(exclude_id_name.size());
    for (unsigned int i = 0; i < exclude_id_name.size(); ++i)
    {
      id_index[i] = mesh->get_elem_integer_index(exclude_id_name[i]);
      std::copy(exclude_id_value[i].begin(),
                exclude_id_value[i].end(),
                std::inserter(id_value_set[i], id_value_set[i].end()));
    }
    // list of unique IDs not removed by "exclude_id_name" and "exclude_id_value" option
    std::set<dof_id_type> ids;
    for (const auto & elem : mesh->active_element_ptr_range())
    {
      // don't need to check if this unique parsed ID corresponding to this element is already in
      // the "ids"
      if (ids.count(parsed_ids[elem->id()]))
        continue;
      // check extra IDs of this element is defined in  "exclude_id_name" and "exclude_id_value"
      bool is_exclude_elem = false;
      for (unsigned int i = 0; i < id_index.size(); ++i)
      {
        const auto id = elem->get_extra_integer(id_index[i]);
        if (id_value_set[i].count(id))
        {
          is_exclude_elem = true;
          break;
        }
      }
      // if this element does not need to be excluded, having unique ID, add to "ids"
      if (!is_exclude_elem)
        ids.insert(parsed_ids[elem->id()]);
    }
    comm().set_union(ids);

    std::map<dof_id_type, dof_id_type> map_ids;
    for (auto id : parsed_ids)
    {
      dof_id_type new_id = std::distance(ids.begin(), ids.find(id.second)) + 1;
      map_ids[id.second] = new_id;
    }

    // reassign parsed (depletion) ids
    for (const auto & elem : mesh->active_element_ptr_range())
    {
      dof_id_type id = parsed_ids[elem->id()];
      dof_id_type new_id = 0;
      if (ids.count(id))
        new_id = map_ids[id];
      parsed_ids[elem->id()] = new_id;
    }
  }
  // assign depletion id to mesh
  const auto depletion_id = mesh->add_elem_integer("depletion_id");
  for (Elem * const elem : mesh->active_element_ptr_range())
    elem->set_extra_integer(depletion_id, parsed_ids.at(elem->id()));
  return mesh;
}
