//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConcentricCircleGeneratorBase.h"

InputParameters
ConcentricCircleGeneratorBase::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRangeCheckedParam<std::vector<Real>>(
      "ring_radii", "ring_radii>0", "Radii of major concentric circles (rings).");
  params.addRangeCheckedParam<std::vector<unsigned int>>(
      "ring_intervals",
      "ring_intervals>0",
      "Number of radial mesh intervals within each major concentric circle excluding their "
      "boundary "
      "layers.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "ring_radial_biases",
      "ring_radial_biases>0",
      "Values used to create biasing in radial meshing for ring regions.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "ring_inner_boundary_layer_widths",
      "ring_inner_boundary_layer_widths>=0",
      "Widths of each ring regions that are assigned to be each ring's inner boundary layers.");
  params.addParam<std::vector<unsigned int>>(
      "ring_inner_boundary_layer_intervals",
      "Number of radial intervals of the rings' inner boundary layers");
  params.addRangeCheckedParam<std::vector<Real>>(
      "ring_inner_boundary_layer_biases",
      "ring_inner_boundary_layer_biases>0",
      "Growth factors used for mesh biasing of the rings' inner boundary layers.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "ring_outer_boundary_layer_widths",
      "ring_outer_boundary_layer_widths>=0",
      "Widths of each ring regions that are assigned to be each ring's outer boundary layers.");
  params.addParam<std::vector<unsigned int>>(
      "ring_outer_boundary_layer_intervals",
      "Number of radial intervals of the rings' outer boundary layers");
  params.addRangeCheckedParam<std::vector<Real>>(
      "ring_outer_boundary_layer_biases",
      "ring_outer_boundary_layer_biases>0",
      "Growth factors used for mesh biasing of the rings' outer boundary layers.");
  params.addParam<std::vector<subdomain_id_type>>(
      "ring_block_ids", "Optional customized block ids for each ring geometry block.");
  params.addParam<std::vector<SubdomainName>>(
      "ring_block_names", "Optional customized block names for each ring geometry block.");
  params.addParam<bool>("preserve_volumes",
                        true,
                        "Volume of concentric circles can be preserved using this function.");
  params.addParam<subdomain_id_type>("block_id_shift", 0, "Integer used to shift block IDs.");
  params.addParam<bool>("create_inward_interface_boundaries",
                        false,
                        "Whether the inward interface boundaries are created.");
  params.addParam<bool>("create_outward_interface_boundaries",
                        true,
                        "Whether the outward interface boundaries are created.");
  params.addParam<boundary_id_type>(
      "interface_boundary_id_shift", 0, "Integer used to shift interface boundary IDs.");
  params.addParam<bool>("generate_side_specific_boundaries",
                        false,
                        "whether the side-specific external boundaries are generated or not");
  params.addRangeCheckedParam<boundary_id_type>("external_boundary_id",
                                                "external_boundary_id>0",
                                                "Optional customized external boundary id.");
  params.addParam<std::string>("external_boundary_name",
                               "Optional customized external boundary name.");
  params.addParam<std::vector<std::string>>(
      "inward_interface_boundary_names",
      "Optional customized boundary names for the internal inward interfaces between block.");
  params.addParam<std::vector<std::string>>(
      "outward_interface_boundary_names",
      "Optional customized boundary names for the internal outward interfaces between block.");

  params.addParamNamesToGroup(
      "ring_block_ids ring_block_names external_boundary_id external_boundary_name "
      "inward_interface_boundary_names outward_interface_boundary_names "
      "block_id_shift create_inward_interface_boundaries create_outward_interface_boundaries "
      "interface_boundary_id_shift generate_side_specific_boundaries",
      "Customized Subdomain/Boundary");
  params.addParamNamesToGroup("ring_intervals", "General Mesh Density");
  params.addParamNamesToGroup(
      "ring_radial_biases ring_inner_boundary_layer_biases ring_inner_boundary_layer_widths "
      "ring_inner_boundary_layer_intervals ring_outer_boundary_layer_biases "
      "ring_outer_boundary_layer_widths ring_outer_boundary_layer_intervals ",
      "Mesh Boundary Layers and Biasing Options");

  params.addClassDescription("This ConcentricCircleGeneratorBase object is a base class to be "
                             "inherited for mesh generators that involve concentric circles.");
  return params;
}

ConcentricCircleGeneratorBase::ConcentricCircleGeneratorBase(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _ring_radii(isParamValid("ring_radii") ? getParam<std::vector<Real>>("ring_radii")
                                           : std::vector<Real>()),
    _ring_intervals(isParamValid("ring_intervals")
                        ? getParam<std::vector<unsigned int>>("ring_intervals")
                        : std::vector<unsigned int>()),
    _ring_radial_biases(isParamValid("ring_radial_biases")
                            ? getParam<std::vector<Real>>("ring_radial_biases")
                            : std::vector<Real>(_ring_intervals.size(), 1.0)),
    _ring_inner_boundary_layer_params(
        {isParamValid("ring_inner_boundary_layer_widths")
             ? getParam<std::vector<Real>>("ring_inner_boundary_layer_widths")
             : std::vector<Real>(_ring_intervals.size(), 0.0),
         std::vector<Real>(),
         isParamValid("ring_inner_boundary_layer_intervals")
             ? getParam<std::vector<unsigned int>>("ring_inner_boundary_layer_intervals")
             : std::vector<unsigned int>(_ring_intervals.size(), 0),
         isParamValid("ring_inner_boundary_layer_biases")
             ? getParam<std::vector<Real>>("ring_inner_boundary_layer_biases")
             : std::vector<Real>(_ring_intervals.size(), 0.0)}),
    _ring_outer_boundary_layer_params(
        {isParamValid("ring_outer_boundary_layer_widths")
             ? getParam<std::vector<Real>>("ring_outer_boundary_layer_widths")
             : std::vector<Real>(_ring_intervals.size(), 0.0),
         std::vector<Real>(),
         isParamValid("ring_outer_boundary_layer_intervals")
             ? getParam<std::vector<unsigned int>>("ring_outer_boundary_layer_intervals")
             : std::vector<unsigned int>(_ring_intervals.size(), 0),
         isParamValid("ring_outer_boundary_layer_biases")
             ? getParam<std::vector<Real>>("ring_outer_boundary_layer_biases")
             : std::vector<Real>(_ring_intervals.size(), 0.0)}),
    _ring_block_ids(isParamValid("ring_block_ids")
                        ? getParam<std::vector<subdomain_id_type>>("ring_block_ids")
                        : std::vector<subdomain_id_type>()),
    _ring_block_names(isParamValid("ring_block_names")
                          ? getParam<std::vector<SubdomainName>>("ring_block_names")
                          : std::vector<SubdomainName>()),
    _preserve_volumes(getParam<bool>("preserve_volumes")),
    _block_id_shift(getParam<subdomain_id_type>("block_id_shift")),
    _create_inward_interface_boundaries(getParam<bool>("create_inward_interface_boundaries")),
    _create_outward_interface_boundaries(getParam<bool>("create_outward_interface_boundaries")),
    _interface_boundary_id_shift(getParam<boundary_id_type>("interface_boundary_id_shift")),
    _generate_side_specific_boundaries(getParam<bool>("generate_side_specific_boundaries")),
    _external_boundary_id(isParamValid("external_boundary_id")
                              ? getParam<boundary_id_type>("external_boundary_id")
                              : 0),
    _external_boundary_name(isParamValid("external_boundary_name")
                                ? getParam<std::string>("external_boundary_name")
                                : std::string()),
    _inward_interface_boundary_names(
        isParamValid("inward_interface_boundary_names")
            ? getParam<std::vector<std::string>>("inward_interface_boundary_names")
            : std::vector<std::string>()),
    _outward_interface_boundary_names(
        isParamValid("outward_interface_boundary_names")
            ? getParam<std::vector<std::string>>("outward_interface_boundary_names")
            : std::vector<std::string>())
{
}

std::unique_ptr<MeshBase>
ConcentricCircleGeneratorBase::generate()
{
  auto mesh = buildReplicatedMesh(2); // initiate a 2D mesh
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
ConcentricCircleGeneratorBase::assignInterfaceBoundaryNames(ReplicatedMesh & mesh)
{
  if (!_inward_interface_boundary_names.empty())
  {
    for (unsigned int i = 0; i < _inward_interface_boundary_names.size(); i++)
    {
      mesh.get_boundary_info().sideset_name(i * 2 + 2 + _interface_boundary_id_shift) =
          _inward_interface_boundary_names[i];
      mesh.get_boundary_info().nodeset_name(i * 2 + 2 + _interface_boundary_id_shift) =
          _inward_interface_boundary_names[i];
    }
  }
  if (!_outward_interface_boundary_names.empty())
  {
    for (unsigned int i = 0; i < _outward_interface_boundary_names.size(); i++)
    {
      mesh.get_boundary_info().sideset_name(i * 2 + 1 + _interface_boundary_id_shift) =
          _outward_interface_boundary_names[i];
      mesh.get_boundary_info().nodeset_name(i * 2 + 1 + _interface_boundary_id_shift) =
          _outward_interface_boundary_names[i];
    }
  }
}
