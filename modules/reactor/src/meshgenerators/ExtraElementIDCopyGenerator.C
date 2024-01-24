//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExtraElementIDCopyGenerator.h"

#include "libmesh/elem.h"

registerMooseObject("ReactorApp", ExtraElementIDCopyGenerator);

InputParameters
ExtraElementIDCopyGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::string>("source_extra_element_id",
                                       "The extra element ID to be copied");
  params.addRequiredParam<std::vector<std::string>>("target_extra_element_ids",
                                                    "The target extra element IDs");
  params.addClassDescription("Copy an extra element ID to other extra element IDs.");
  return params;
}

ExtraElementIDCopyGenerator::ExtraElementIDCopyGenerator(const InputParameters & params)
  : MeshGenerator(params), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
ExtraElementIDCopyGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  auto src_id_name = getParam<std::string>("source_extra_element_id");
  bool copy_subdomain_id = (src_id_name == "subdomain_id");
  bool copy_element_id = (src_id_name == "element_id");
  unsigned int src_id = 0;
  if (!copy_subdomain_id && !copy_element_id)
  {
    if (!mesh->has_elem_integer(src_id_name))
      mooseError("The source element ID does not exist on the input mesh");
    src_id = mesh->get_elem_integer_index(src_id_name);
  }

  auto target_id_names = getParam<std::vector<std::string>>("target_extra_element_ids");

  std::vector<unsigned int> target_ids;
  for (auto & name : target_id_names)
  {
    if (!mesh->has_elem_integer(name))
      mesh->add_elem_integer(name);

    target_ids.push_back(mesh->get_elem_integer_index(name));
  }

  for (auto & elem : mesh->element_ptr_range())
  {
    dof_id_type id;
    if (copy_subdomain_id)
      id = elem->subdomain_id();
    else if (copy_element_id)
      id = elem->id();
    else
      id = elem->get_extra_integer(src_id);

    for (auto & target_id : target_ids)
      elem->set_extra_integer(target_id, id);
  }

  return mesh;
}
