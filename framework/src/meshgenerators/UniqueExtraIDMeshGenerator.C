//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UniqueExtraIDMeshGenerator.h"
#include "MooseMeshUtils.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", UniqueExtraIDMeshGenerator);

InputParameters
UniqueExtraIDMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>(
      "input", "Name of an existing mesh generator to which we assign extra element IDs");
  params.addRequiredParam<std::vector<ExtraElementIDName>>(
      "id_name",
      "Existing extra integer ID names that is used to generate a new extra integer ID by finding "
      "unique combinations of their values");
  params.addRequiredParam<ExtraElementIDName>("new_id_name", "New extra integer ID name");
  params.addParam<std::vector<unsigned int>>(
      "new_id_rule",
      "Vector of unsigned integers to determine new integer ID values by multiplying the provided "
      "integers to the corresponding existing ID values and then summing the resulting values");
  params.addClassDescription("Add a new extra element integer ID by finding unique combinations of "
                             "the existing extra element integer ID values");
  return params;
}

UniqueExtraIDMeshGenerator::UniqueExtraIDMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _extra_ids(getParam<std::vector<ExtraElementIDName>>("id_name")),
    _use_new_id_rule(isParamValid("new_id_rule"))
{
  if (_use_new_id_rule &&
      getParam<std::vector<unsigned int>>("new_id_rule").size() != _extra_ids.size())
    paramError("new_id_rule",
               "This parameter, if provided, must have a length equal to length of id_name.");
}

std::unique_ptr<MeshBase>
UniqueExtraIDMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  auto parsed_ids = MooseMeshUtils::getExtraIDUniqueCombinationMap(*mesh, {}, _extra_ids);

  // override the extra ID values from MooseMeshUtils::getExtraIDUniqueCombinationMap by using
  // new_id_rule
  if (_use_new_id_rule)
  {
    std::vector<unsigned int> new_id_rule = getParam<std::vector<unsigned int>>("new_id_rule");
    std::vector<unsigned int> existing_extra_id_index;
    for (const auto & id_name : _extra_ids)
      existing_extra_id_index.push_back(mesh->get_elem_integer_index(id_name));
    for (auto & elem : mesh->active_local_element_ptr_range())
    {
      dof_id_type new_id_value = 0;
      for (unsigned int i = 0; i < _extra_ids.size(); ++i)
        new_id_value += new_id_rule[i] * elem->get_extra_integer(existing_extra_id_index[i]);
      parsed_ids[elem->id()] = new_id_value;
    }
  }
  auto new_id_name = getParam<ExtraElementIDName>("new_id_name");
  unsigned int extra_id_index;
  if (!mesh->has_elem_integer(new_id_name))
    extra_id_index = mesh->add_elem_integer(new_id_name);
  else
  {
    extra_id_index = mesh->get_elem_integer_index(new_id_name);
    paramWarning(
        "new_id_name", "An element integer with the name '", new_id_name, "' already exists");
  }
  for (auto & elem : mesh->active_element_ptr_range())
    elem->set_extra_integer(extra_id_index, parsed_ids.at(elem->id()));
  return mesh;
}
