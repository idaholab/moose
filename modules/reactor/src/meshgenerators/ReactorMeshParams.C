//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactorMeshParams.h"
#include "CastUniquePointer.h"
#include "ReactorGeometryMeshBuilderBase.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"

registerMooseObject("ReactorApp", ReactorMeshParams);

InputParameters
ReactorMeshParams::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  MooseEnum dims("2=2 3", "2");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

  MooseEnum geoms("Square Hex", "Square");
  params.addRequiredParam<MooseEnum>("geom", geoms, "The geometry type of the reactor mesh");

  params.addRequiredParam<Real>("assembly_pitch", "Center to center distance of assemblies");
  params.addParam<boundary_id_type>("top_boundary_id",
                                    "The boundary ID to set on top boundary of the extruded mesh");
  params.addParam<boundary_id_type>(
      "bottom_boundary_id", "The boundary ID to set on bottom boundary of the extruded mesh");
  params.addParam<boundary_id_type>(
      "radial_boundary_id",
      "The boundary ID to set on the outer radial boundary of a CoreMeshGenerator object");
  params.addParam<std::vector<Real>>("axial_regions", "Length of each axial region");
  params.addParam<std::vector<unsigned int>>(
      "axial_mesh_intervals",
      "Number of elements in the Z direction for each axial region");
  params.addParam<bool>("region_id_as_block_name", false, "Set block names based on region id");
  params.addParam<bool>(
      "flexible_assembly_stitching",
      false,
      "Use FlexiblePatternGenerator for stitching dissimilar assemblies together");
  params.addRangeCheckedParam<unsigned int>(
      "num_sectors_at_flexible_boundary",
      6,
      "num_sectors_at_flexible_boundary>2",
      "Number of sectors to use at assembly boundary interface when flexible patterning is used "
      "(Defaults to 6)");
  params.addClassDescription("This ReactorMeshParams object acts as storage for persistent "
                             "information about the reactor geometry.");

  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  return params;
}

ReactorMeshParams::ReactorMeshParams(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _geom(getParam<MooseEnum>("geom")),
    _assembly_pitch(getParam<Real>("assembly_pitch"))
{
  if ((unsigned int)(_dim) == 2)
  {
    std::vector<std::string> invalid_params = {
        "axial_regions", "axial_mesh_intervals", "top_boundary_id", "bottom_boundary_id"};
    for (const auto & param : invalid_params)
      if (isParamValid(param))
        paramError(param, param + " should not be defined for 2-D meshes");
  }
  else
  {
    _axial_regions = getParam<std::vector<Real>>("axial_regions");
    _axial_mesh_intervals = getParam<std::vector<unsigned int>>("axial_mesh_intervals");

    if (_axial_regions.size() != _axial_mesh_intervals.size())
      mooseError(
          "The number of axial regions is not consistent with the number of axial intervals.");
    this->declareMeshProperty(RGMB::axial_mesh_sizes, _axial_regions);
    this->declareMeshProperty(RGMB::axial_mesh_intervals, _axial_mesh_intervals);
  }

  this->declareMeshProperty(RGMB::mesh_dimensions, (unsigned int)std::stoul(_dim));
  this->declareMeshProperty(RGMB::mesh_geometry, std::string(_geom));
  this->declareMeshProperty(RGMB::assembly_pitch, _assembly_pitch);
  this->declareMeshProperty(RGMB::region_id_as_block_name,
                            getParam<bool>(RGMB::region_id_as_block_name));

  const bool flexible_assembly_stitching = getParam<bool>(RGMB::flexible_assembly_stitching);
  this->declareMeshProperty(RGMB::flexible_assembly_stitching, flexible_assembly_stitching);
  if (flexible_assembly_stitching)
    this->declareMeshProperty(RGMB::num_sectors_flexible_stitching,
                              getParam<unsigned int>("num_sectors_at_flexible_boundary"));
  if (parameters.isParamSetByUser("num_sectors_at_flexible_boundary") &&
      !flexible_assembly_stitching)
    paramWarning(
        "num_sectors_at_flexible_boundary",
        "This parameter is only relevant when ReactorMeshParams/flexible_assembly_stitching is set "
        "to true. This value will be ignored");

  // Option to bypass mesh generation is controlled by presence of Mesh/data_driven_generator
  // and whether the current generator is in data only mode
  const auto & moose_mesh = _app.actionWarehouse().getMesh();
  const auto data_driven_generator =
      moose_mesh->parameters().get<std::string>("data_driven_generator");
  bool bypass_meshgen = (data_driven_generator != "") && isDataOnly();
  this->declareMeshProperty(RGMB::bypass_meshgen, bypass_meshgen);

  if (isParamValid("top_boundary_id"))
  {
    _top_boundary = getParam<boundary_id_type>("top_boundary_id");
    this->declareMeshProperty(RGMB::top_boundary_id, _top_boundary);
  }
  if (isParamValid("bottom_boundary_id"))
  {
    _bottom_boundary = getParam<boundary_id_type>("bottom_boundary_id");
    this->declareMeshProperty(RGMB::bottom_boundary_id, _bottom_boundary);
  }
  if (isParamValid("radial_boundary_id"))
  {
    _radial_boundary = getParam<boundary_id_type>("radial_boundary_id");
    this->declareMeshProperty(RGMB::radial_boundary_id, _radial_boundary);
    if (isParamValid("top_boundary_id") && _radial_boundary == _top_boundary)
      mooseError("top_boundary_id and radial_boundary_id must be unique values");
    if (isParamValid("bottom_boundary_id") && _radial_boundary == _bottom_boundary)
      mooseError("bottom_boundary_id and radial_boundary_id must be unique values");
  }
  if (isParamValid("top_boundary_id") && isParamValid("bottom_boundary_id") &&
      (_bottom_boundary == _top_boundary))
    mooseError("top_boundary_id and bottom_boundary_id must be unique values");
}

std::unique_ptr<MeshBase>
ReactorMeshParams::generate()
{
  // If mesh generation is requested and bypass_mesh is true, return a null mesh. generate()
  // mesh should not be called with this option specified
  if (getMeshProperty<bool>(RGMB::bypass_meshgen))
  {
    auto null_mesh = nullptr;
    return null_mesh;
  }
  auto mesh = buildMeshBaseObject();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
