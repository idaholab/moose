//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CartesianIDPatternedMeshGenerator.h"
#include "libmesh/elem.h"

registerMooseObject("ReactorApp", CartesianIDPatternedMeshGenerator);

InputParameters
CartesianIDPatternedMeshGenerator::validParams()
{
  InputParameters params = PatternedMeshGenerator::validParams();
  params.addRequiredParam<std::string>("id_name", "Name of integer ID set");
  params.addParam<std::vector<MeshGeneratorName>>(
      "exclude_id", "Name of input meshes to be excluded in ID generation");
  MooseEnum option("cell pattern manual", "cell");
  params.addParam<MooseEnum>("assign_type", option, "Type of integer ID assignment");
  params.addParam<std::vector<std::vector<dof_id_type>>>(
      "id_pattern",
      "User-defined element IDs. A double-indexed array starting with the upper-left corner");
  params.addClassDescription("Generate Certesian lattice meshes with reporting ID assignment that "
                             "indentifies individual components of lattice.");
  return params;
}

CartesianIDPatternedMeshGenerator::CartesianIDPatternedMeshGenerator(
    const InputParameters & parameters)
  : PatternedMeshGenerator(parameters),
    _element_id_name(getParam<std::string>("id_name")),
    _assign_type(getParam<MooseEnum>("assign_type")),
    _use_exclude_id(isParamValid("exclude_id"))
{
  if (_use_exclude_id && _assign_type != "cell")
    paramError("exclude_id", "works only when \"assign_type\" is equal 'cell'");
  if (!isParamValid("id_pattern") && _assign_type == "manual")
    paramError("id_pattern", "required when \"assign_type\" is equal to 'manual'");

  if (_assign_type == "manual")
    _id_pattern = getParam<std::vector<std::vector<dof_id_type>>>("id_pattern");
  _exclude_ids.resize(_input_names.size());
  if (_use_exclude_id)
  {
    std::vector<MeshGeneratorName> exclude_id_name =
        getParam<std::vector<MeshGeneratorName>>("exclude_id");
    for (unsigned int i = 0; i < _input_names.size(); ++i)
    {
      _exclude_ids[i] = false;
      for (auto input_name : exclude_id_name)
        if (_input_names[i] == input_name)
        {
          _exclude_ids[i] = true;
          break;
        }
    }
  }
  else
    for (unsigned int i = 0; i < _input_names.size(); ++i)
      _exclude_ids[i] = false;
}

std::unique_ptr<MeshBase>
CartesianIDPatternedMeshGenerator::generate()
{
  auto mesh = PatternedMeshGenerator::generate();
  // assumes that the entire mesh has elements of each individual mesh sequentially ordered.
  std::vector<dof_id_type> integer_ids;
  if (_assign_type == "cell")
    integer_ids = getCellwiseIntegerIDs();
  else if (_assign_type == "pattern")
    integer_ids = getPatternIntegerIDs();
  else if (_assign_type == "manual")
    integer_ids = getManualIntegerIDs();

  unsigned int extra_id_index = 0;
  if (!mesh->has_elem_integer(_element_id_name))
    extra_id_index = mesh->add_elem_integer(_element_id_name);
  else
    extra_id_index = mesh->get_elem_integer_index(_element_id_name);
  unsigned int i = 0;
  for (auto & elem : mesh->element_ptr_range())
    elem->set_extra_integer(extra_id_index, integer_ids[i++]);
  return mesh;
}

std::vector<dof_id_type>
CartesianIDPatternedMeshGenerator::getCellwiseIntegerIDs() const
{
  std::vector<dof_id_type> integer_ids;
  dof_id_type id = 0;
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
  {
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
    {
      const ReplicatedMesh & cell_mesh = *_meshes[_pattern[i][j]];
      unsigned int n_cell_elem = cell_mesh.n_elem();
      bool exclude_id = false;
      if (_use_exclude_id)
        if (_exclude_ids[_pattern[i][j]])
          exclude_id = true;
      if (!exclude_id)
      {
        for (unsigned int k = 0; k < n_cell_elem; ++k)
          integer_ids.push_back(id);
        ++id;
      }
      else
      {
        for (unsigned int k = 0; k < n_cell_elem; ++k)
          integer_ids.push_back(DofObject::invalid_id);
      }
    }
  }
  return integer_ids;
}

std::vector<dof_id_type>
CartesianIDPatternedMeshGenerator::getPatternIntegerIDs() const
{
  std::vector<dof_id_type> integer_ids;
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
  {
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
    {
      const ReplicatedMesh & cell_mesh = *_meshes[_pattern[i][j]];
      unsigned int n_cell_elem = cell_mesh.n_elem();
      for (unsigned int k = 0; k < n_cell_elem; ++k)
        integer_ids.push_back(_pattern[i][j]);
    }
  }
  return integer_ids;
}

std::vector<dof_id_type>
CartesianIDPatternedMeshGenerator::getManualIntegerIDs() const
{
  std::vector<dof_id_type> integer_ids;
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
  {
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
    {
      dof_id_type id = _id_pattern[i][j];
      const ReplicatedMesh & cell_mesh = *_meshes[_pattern[i][j]];
      unsigned int n_cell_elem = cell_mesh.n_elem();
      for (unsigned int k = 0; k < n_cell_elem; ++k)
        integer_ids.push_back(id);
    }
  }
  return integer_ids;
}
