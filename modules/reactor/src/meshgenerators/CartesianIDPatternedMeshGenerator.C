//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CartesianIDPatternedMeshGenerator.h"
#include "ReportingIDGeneratorUtils.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_tools.h"

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

  // Build a per-tile ID lookup from the utility helpers.  The flat integer_ids vectors
  // returned by the helpers contain n_elem(tile) identical values per tile in row-major
  // order; here we only need the representative value for each (row, col) position.
  const auto bbox = MeshTools::create_bounding_box(*_meshes[0]);
  const Real min_x = bbox.min()(0);
  const Real min_y = bbox.min()(1);

  // Populate tile_id[i][j] using the appropriate utility helper.
  std::vector<std::vector<dof_id_type>> tile_id(_pattern.size());
  if (_assign_type == ReportingIDGeneratorUtils::AssignType::cell)
  {
    const auto integer_ids = ReportingIDGeneratorUtils::getCellwiseIntegerIDs(
        _meshes, _pattern, _use_exclude_id, _exclude_ids);
    dof_id_type k = 0;
    for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
    {
      tile_id[i].resize(_pattern[i].size());
      for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
      {
        tile_id[i][j] = integer_ids[k];
        k += _meshes[_pattern[i][j]]->n_elem();
      }
    }
  }
  else if (_assign_type == ReportingIDGeneratorUtils::AssignType::pattern)
  {
    const auto integer_ids = ReportingIDGeneratorUtils::getPatternIntegerIDs(_meshes, _pattern);
    dof_id_type k = 0;
    for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
    {
      tile_id[i].resize(_pattern[i].size());
      for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
      {
        tile_id[i][j] = integer_ids[k];
        k += _meshes[_pattern[i][j]]->n_elem();
      }
    }
  }
  else // manual
  {
    const auto integer_ids =
        ReportingIDGeneratorUtils::getManualIntegerIDs(_meshes, _pattern, _id_pattern);
    dof_id_type k = 0;
    for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
    {
      tile_id[i].resize(_pattern[i].size());
      for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
      {
        tile_id[i][j] = integer_ids[k];
        k += _meshes[_pattern[i][j]]->n_elem();
      }
    }
  }

  // Assign by centroid so the result is correct regardless of element iteration order.
  // This is necessary in distributed mode where element_ptr_range is not tile-major.
  libMesh::MeshSerializer mesh_serializer(*mesh);
  for (auto & elem : mesh->element_ptr_range())
  {
    const auto c = elem->vertex_average();
    const int col = static_cast<int>((c(0) - min_x) / _x_width);
    const int row = static_cast<int>((min_y + _y_width - c(1)) / _y_width);
    elem->set_extra_integer(extra_id_index, tile_id[row][col]);
  }

  return mesh;
}
