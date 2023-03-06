//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CartesianIDPatternedMeshGenerator.h"
#include "ReportingIDGeneratorUtils.h"

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
    _assign_type(
        getParam<MooseEnum>("assign_type").getEnum<ReportingIDGeneratorUtils::AssignType>()),
    _use_exclude_id(isParamValid("exclude_id"))
{
  if (_use_exclude_id && _assign_type != ReportingIDGeneratorUtils::AssignType::cell)
    paramError("exclude_id", "works only when \"assign_type\" is equal 'cell'");
  if (!isParamValid("id_pattern") && _assign_type == ReportingIDGeneratorUtils::AssignType::manual)
    paramError("id_pattern", "required when \"assign_type\" is equal to 'manual'");

  if (_assign_type == ReportingIDGeneratorUtils::AssignType::manual)
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

  unsigned int extra_id_index;
  if (!mesh->has_elem_integer(_element_id_name))
    extra_id_index = mesh->add_elem_integer(_element_id_name);
  else
  {
    extra_id_index = mesh->get_elem_integer_index(_element_id_name);
    paramWarning(
        "id_name", "An element integer with the name '", _element_id_name, "' already exists");
  }

  // patternedMeshGenerator for Carterisan lattice does not support duct structures
  const bool has_assembly_duct = false;
  const std::set<subdomain_id_type> duct_block_ids;
  // asssign reporting IDs to individual elements
  ReportingIDGeneratorUtils::assignReportingIDs(mesh,
                                                extra_id_index,
                                                _assign_type,
                                                _use_exclude_id,
                                                _exclude_ids,
                                                has_assembly_duct,
                                                duct_block_ids,
                                                _meshes,
                                                _pattern,
                                                _id_pattern);

  return mesh;
}
