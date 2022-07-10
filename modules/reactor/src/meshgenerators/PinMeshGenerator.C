//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PinMeshGenerator.h"

#include <cmath>
#include "MooseApp.h"
#include "Factory.h"
#include "libmesh/elem.h"

registerMooseObject("ReactorApp", PinMeshGenerator);

InputParameters
PinMeshGenerator::validParams()
{
  auto params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "reactor_params",
      "The ReactorMeshParams MeshGenerator that is the basis for this component conformal mesh.");

  params.addRequiredParam<subdomain_id_type>("pin_type",
                                             "The integer ID for this pin type definition");

  params.addRequiredRangeCheckedParam<Real>(
      "pitch", "pitch>0.0", "The pitch for the outermost boundary polygon");

  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_sectors", "num_sectors>0", "Number of azimuthal sectors in each quadrant");

  params.addRangeCheckedParam<std::vector<Real>>(
      "ring_radii",
      "ring_radii>0.0",
      "Radii of major concentric circles of the pin. If unspecified, no pin is present.");

  params.addRangeCheckedParam<std::vector<Real>>(
      "duct_halfpitch",
      "duct_halfpitch>0.0",
      "Apothem of the ducts. If unspecified, no duct is present.");

  params.addRangeCheckedParam<std::vector<unsigned int>>(
      "mesh_intervals",
      std::vector<unsigned int>{1},
      "mesh_intervals>0",
      "The number of meshing intervals for each region starting at the center. Parameter should be "
      "size:"
      "((length(ring_radii) + length(duct_halfpitch) + 1");

  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "block_names",
      "Block names for each radial and axial zone. "
      "Inner indexing is radial zones (pin/background/duct), outer indexing is axial");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "region_ids",
      "IDs for each radial and axial zone for assignment of region_id extra element "
      "id. "
      "Inner indexing is radial zones (pin/background/duct), outer indexing is axial");

  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building in the Reactor workflow");

  params.addParam<bool>(
      "quad_center_elements", true, "Whether the center elements are quad or triangular.");
  params.addParamNamesToGroup("region_ids pin_type", "ID assigment");
  params.addParamNamesToGroup("mesh_intervals ring_radii num_sectors pin_type",
                              "Pin specifications");
  params.addParamNamesToGroup("mesh_intervals duct_halfpitch num_sectors", "Duct specifications");

  params.addClassDescription("This PinMeshGenerator object is designed to generate pin-like "
                             "structures, with IDs, from a reactor geometry. "
                             "Whether it be a square or hexagonal pin, they are divided into three "
                             "substructures - the innermost "
                             "radial pin regions, the single bridging background region, and the "
                             "square or hexagonal ducts regions.");

  return params;
}

PinMeshGenerator::PinMeshGenerator(const InputParameters & parameters)
  : ReactorGeometryMeshBuilderBase(parameters),
    _pin_type(getParam<subdomain_id_type>("pin_type")),
    _pitch(getParam<Real>("pitch")),
    _num_sectors(getParam<unsigned int>("num_sectors")),
    _ring_radii(isParamValid("ring_radii") ? getParam<std::vector<Real>>("ring_radii")
                                           : std::vector<Real>()),
    _duct_halfpitch(isParamValid("duct_halfpitch") ? getParam<std::vector<Real>>("duct_halfpitch")
                                                   : std::vector<Real>()),
    _intervals(getParam<std::vector<unsigned int>>("mesh_intervals")),
    _region_ids(isParamValid("region_ids")
                    ? getParam<std::vector<std::vector<subdomain_id_type>>>("region_ids")
                    : std::vector<std::vector<subdomain_id_type>>()),
    _extrude(getParam<bool>("extrude")),
    _quad_center(getParam<bool>("quad_center_elements"))
{
  // Initialize name_id_map and current_block_id from ReactorMeshParams object
  initializeReactorMeshParams(getParam<MeshGeneratorName>("reactor_params"));

  _mesh_dimensions = getReactorParam<int>("mesh_dimensions");
  _mesh_geometry = getReactorParam<std::string>("mesh_geometry");

  if (_extrude && _mesh_dimensions != 3)
    mooseError("This is a 2 dimensional mesh, you cannot extrude it. Check your ReactorMeshParams "
               "inputs\n");
  if (_extrude && (!hasReactorParam("top_boundary_id") ||
                   !hasReactorParam("bottom_boundary_id")))
    mooseError("Both top_boundary_id and bottom_boundary_id must be provided in ReactorMeshParams "
               "if using extruded geometry");

  if (_intervals.size() != (_ring_radii.size() + _duct_halfpitch.size() + 1))
    mooseError("The number of mesh intervals must be equal to the number of annular regions + the "
               "number of duct regions + 1"
               " for the region between the rings and ducts\n");

  if (isParamValid("region_ids"))
  {
    if (_region_ids[0].size() != (_ring_radii.size() + _duct_halfpitch.size() + 1))
      mooseError("The number of region IDs given needs to be one more than the number of "
                 "ring_radii + the number of duct_radii\n");
  }
  else
  {
    mooseError("Region IDs must be assigned with parameter region_ids");
  }
  if (isParamValid("block_names"))
  {
    _has_block_names = true;
    _block_names = getParam<std::vector<std::vector<SubdomainName>>>("block_names");
    if (_region_ids.size() != _block_names.size())
        mooseError("The size of block_names must match the size of region_ids");
    for (const auto i : make_range(_region_ids.size()))
      if (_region_ids[i].size() != _block_names[i].size())
        mooseError("The size of block_names must match the size of region_ids");
  }
  else
    _has_block_names = false;

  // Define all id variables used in the pin
  std::vector<unsigned int> ring_intervals;
  std::vector<subdomain_id_type> ring_blk_ids;
  std::vector<SubdomainName> ring_blk_names;
  unsigned int background_intervals = 1;
  std::vector<subdomain_id_type> background_blk_ids;
  std::vector<SubdomainName> background_blk_names;
  std::vector<unsigned int> duct_intervals;
  std::vector<subdomain_id_type> duct_blk_ids;
  std::vector<SubdomainName> duct_blk_names;

  for (const auto i : make_range(_intervals.size()))
  {
    const auto region_id = _region_ids[0][i];
    const auto block_name = (_has_block_names ? _block_names[0][i] :
                             _block_name_prefix + std::to_string(region_id));
    const auto block_id = getBlockId(block_name, region_id);

    if (i < _ring_radii.size())
    {
      ring_intervals.push_back(_intervals[i]);
      ring_blk_ids.push_back(block_id);
      ring_blk_names.push_back(block_name);
    }
    else if (i > _ring_radii.size())
    {
      duct_intervals.push_back(_intervals[i]);
      duct_blk_ids.push_back(block_id);
      duct_blk_names.push_back(block_name);
    }
    else
    {
      background_intervals = _intervals[i];
      background_blk_ids.push_back(block_id);
      background_blk_names.push_back(block_name);
    }
  }
  if (ring_intervals.size() > 0)
  {
    if (ring_intervals.front() != 1)
    {
      // If quad center elements, copy element at beginning of block names and
      // block ids. Otherwise add "_TRI" suffix to block names and generate new
      // block id
      if (_quad_center)
      {
        ring_blk_ids.insert(ring_blk_ids.begin(), ring_blk_ids.front());
        ring_blk_names.insert(ring_blk_names.begin(), ring_blk_names.front());
      }
      else
      {
        const auto region_id = _region_ids[0][0];
        const auto block_name = ring_blk_names.front() + "_TRI";
        const auto block_id = getBlockId(block_name, region_id);
        ring_blk_ids.insert(ring_blk_ids.begin(), block_id);
        ring_blk_names.insert(ring_blk_names.begin(), block_name);
      }
    }
  }
  else if (background_intervals > 1)
  {
    // If quad center elements, copy element at beginning of block names and
    // block ids. Otherwise add "_TRI" suffix to block names and generate new
    // block id
    if (_quad_center)
    {
      background_blk_ids.insert(background_blk_ids.begin(), background_blk_ids.front());
      background_blk_names.insert(background_blk_names.begin(), background_blk_names.front());
    }
    else
    {
      const auto region_id = _region_ids[0][_ring_radii.size()];
      const auto block_name = background_blk_names.front() + "_TRI";
      const auto block_id = getBlockId(block_name, region_id);
      background_blk_ids.insert(background_blk_ids.begin(), block_id);
      background_blk_names.insert(background_blk_names.begin(), block_name);
    }
  }

  // Generate Cartesian/hex pin using PolygonConcentricCircleMeshGenerator
  {
    // Get and assign parameters for the main geometry feature of the Pin
    // which is created with a PolygonConcentricCircleMeshGenerator subgenerator
    auto params = _app.getFactory().getValidParams("PolygonConcentricCircleMeshGenerator");
    params.set<bool>("preserve_volumes") = true;
    params.set<bool>("quad_center_elements") = _quad_center;
    params.set<MooseEnum>("polygon_size_style") = "apothem";
    params.set<Real>("polygon_size") = _pitch / 2.0;
    params.set<boundary_id_type>("external_boundary_id") = 20000 + _pin_type;
    params.set<std::string>("external_boundary_name") = "outer_pin_" + std::to_string(_pin_type);
    params.set<bool>("flat_side_up") = true;

    const auto num_sides = (_mesh_geometry == "Square") ? 4 : 6;
    params.set<unsigned int>("num_sides") = num_sides;
    params.set<std::vector<unsigned int>>("num_sectors_per_side") =
        std::vector<unsigned int>(num_sides, _num_sectors);

    params.set<boundary_id_type>("interface_boundary_id_shift") =
        30000; // need to shift interface boundaries to avoid clashing
               // with default IDs for PatternedMeshGenerator

    if (ring_intervals.size() > 0)
    {
      params.set<std::vector<Real>>("ring_radii") = _ring_radii;
      params.set<std::vector<subdomain_id_type>>("ring_block_ids") = ring_blk_ids;
      params.set<std::vector<SubdomainName>>("ring_block_names") = ring_blk_names;
      params.set<std::vector<unsigned int>>("ring_intervals") = ring_intervals;
    }

    params.set<std::vector<subdomain_id_type>>("background_block_ids") = background_blk_ids;
    params.set<std::vector<SubdomainName>>("background_block_names") = background_blk_names;
    params.set<unsigned int>("background_intervals") = background_intervals;

    if (duct_intervals.size() > 0)
    {
      params.set<MooseEnum>("duct_sizes_style") = "apothem";
      params.set<std::vector<Real>>("duct_sizes") = _duct_halfpitch;
      params.set<std::vector<subdomain_id_type>>("duct_block_ids") = duct_blk_ids;
      params.set<std::vector<SubdomainName>>("duct_block_names") = duct_blk_names;
      params.set<std::vector<unsigned int>>("duct_intervals") = duct_intervals;
    }

    addMeshSubgenerator("PolygonConcentricCircleMeshGenerator", name() + "_2D", params);
  }

  // Remove extra sidesets created by PolygonConcentricCircleMeshGenerator
  {
    auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_2D";

    auto num_sides = (_mesh_geometry == "Square") ? 4 : 6;
    std::vector<BoundaryName> boundaries_to_delete = {};
    for (int i = 0; i < num_sides; i++)
      boundaries_to_delete.insert(boundaries_to_delete.end(),
                                  {std::to_string(10001 + i), std::to_string(15001 + i)});
    params.set<std::vector<BoundaryName>>("boundary_names") = boundaries_to_delete;

    _build_mesh = &addMeshSubgenerator("BoundaryDeletionGenerator", name() + "_del_bds", params);
  }

  // Pass mesh meta-data from subgenerator to this MeshGenerator
  if (hasMeshProperty("pitch_meta", name() + "_2D"))
    declareMeshProperty("pitch_meta", getMeshProperty<Real>("pitch_meta", name() + "_2D"));
  if (hasMeshProperty("background_intervals_meta", name() + "_2D"))
    declareMeshProperty("background_intervals_meta",
                        getMeshProperty<unsigned int>("background_intervals_meta", name() + "_2D"));
  if (hasMeshProperty("node_id_background_meta", name() + "_2D"))
    declareMeshProperty("node_id_background_meta",
                        getMeshProperty<unsigned int>("node_id_background_meta", name() + "_2D"));
  if (hasMeshProperty("pattern_pitch_meta", name() + "_2D"))
    declareMeshProperty("pattern_pitch_meta",
                        getMeshProperty<Real>("pattern_pitch_meta", name() + "_2D"));
  if (hasMeshProperty("azimuthal_angle_meta", name() + "_2D"))
    declareMeshProperty("azimuthal_angle_meta",
                        getMeshProperty<std::vector<Real>>("azimuthal_angle_meta", name() + "_2D"));
  if (hasMeshProperty("num_sectors_per_side_meta", name() + "_2D"))
    declareMeshProperty(
        "num_sectors_per_side_meta",
        getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta", name() + "_2D"));
  if (hasMeshProperty("max_radius_meta", name() + "_2D"))
    declareMeshProperty("max_radius_meta",
                        getMeshProperty<Real>("max_radius_meta", name() + "_2D"));

  // id swap info after extrusion
  std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> id_map{
      {_pin_type, _region_ids}};
  declareMeshProperty("pin_region_ids", id_map);

  if (_extrude && _mesh_dimensions == 3)
  {
    std::vector<Real> axial_boundaries =
        getReactorParam<std::vector<Real>>("axial_boundaries");
    const auto top_boundary = getReactorParam<boundary_id_type>("top_boundary_id");
    const auto bottom_boundary =
        getReactorParam<boundary_id_type>("bottom_boundary_id");
    {
      declareMeshProperty("extruded", true);
      auto params = _app.getFactory().getValidParams("FancyExtruderGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_del_bds";
      params.set<Point>("direction") = Point(0, 0, 1);
      params.set<std::vector<unsigned int>>("num_layers") =
          getReactorParam<std::vector<unsigned int>>("axial_mesh_intervals");
      params.set<std::vector<Real>>("heights") = axial_boundaries;
      params.set<boundary_id_type>("bottom_boundary") = bottom_boundary;
      params.set<boundary_id_type>("top_boundary") = top_boundary;
      addMeshSubgenerator("FancyExtruderGenerator", name() + "_extruded", params);
    }

    {
      auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_extruded";
      params.set<std::vector<BoundaryName>>("old_boundary") = {
          std::to_string(top_boundary),
          std::to_string(bottom_boundary)}; // hard coded boundary IDs in patterned mesh generator
      params.set<std::vector<BoundaryName>>("new_boundary") = {"top", "bottom"};

      addMeshSubgenerator("RenameBoundaryGenerator", name() + "_change_plane_name", params);
    }

    {
      auto params = _app.getFactory().getValidParams("PlaneIDMeshGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_change_plane_name";

      std::vector<Real> plane_heights{0};
      for (Real z : axial_boundaries)
        plane_heights.push_back(z + plane_heights.back());

      params.set<std::vector<Real>>("plane_coordinates") = plane_heights;

      std::string plane_id_name = "plane_id";
      params.set<std::string>("id_name") = "plane_id";

      _build_mesh = &addMeshSubgenerator("PlaneIDMeshGenerator", name() + "_extrudedIDs", params);
    }
  }
  else
    declareMeshProperty("extruded", false);

  // Save updates to name id map to ReactorMeshParams object
  updateReactorMeshParams();
}

std::unique_ptr<MeshBase>
PinMeshGenerator::generate()
{
  // Add region id and pin type id to the mesh as element integers
  std::string region_id_name = "region_id";
  std::string pin_type_id_name = "pin_type_id";
  std::string plane_id_name = "plane_id";
  unsigned int rid, ptid, pid;

  if (!(*_build_mesh)->has_elem_integer(region_id_name))
    rid = (*_build_mesh)->add_elem_integer(region_id_name);
  else
    rid = (*_build_mesh)->get_elem_integer_index(region_id_name);
  if (!(*_build_mesh)->has_elem_integer(pin_type_id_name))
    ptid = (*_build_mesh)->add_elem_integer(pin_type_id_name);
  else
    ptid = (*_build_mesh)->get_elem_integer_index(pin_type_id_name);
  if (_extrude)
    pid = (*_build_mesh)->get_elem_integer_index(plane_id_name);

  // Get region id and block information of elements defined by RGMB so far
  const auto name_id_map = getReactorParam<std::map<std::string, std::pair<subdomain_id_type, dof_id_type>>>("name_id_map");
  for (auto it = name_id_map.begin(); it != name_id_map.end(); ++it)
  {
    const auto base_block_id = it->second.first;
    subdomain_id_type base_region_id = it->second.second;
    const auto base_block_name = it->first;
    const auto region_id_idx = std::find(_region_ids[0].begin(), _region_ids[0].end(), base_region_id) - _region_ids[0].begin();

    // Loop through all elements by subdomain id and set extra element integers.
    // Also swap the region ids on the subdomain ids to the correct ones for their
    // axial layer if necessary
    for (const auto & elem : as_range((*_build_mesh)->active_subdomain_elements_begin(base_block_id),
                                      (*_build_mesh)->active_subdomain_elements_end(base_block_id)))
    {
      dof_id_type z_id = _extrude ? elem->get_extra_integer(pid) : 0;

      const subdomain_id_type elem_region_id = _region_ids[std::size_t(z_id)][region_id_idx];
      // Set element integers
      elem->set_extra_integer(rid, elem_region_id);
      elem->set_extra_integer(ptid, _pin_type);

      // Update block ids and names if they differ axially
      if (elem_region_id != base_region_id)
      {
        auto elem_block_name = (_has_block_names ? _block_names[std::size_t(z_id)][region_id_idx] :
                                _block_name_prefix + std::to_string(elem_region_id));
        if (elem->type() == TRI3 || elem->type() == PRISM6)
          elem_block_name += "_TRI";
        const auto elem_block_id = getBlockId(elem_block_name, elem_region_id);
        elem->subdomain_id() = elem_block_id;
        (*_build_mesh)->subdomain_name(elem_block_id) = elem_block_name;
      }
    }
  }

  // Update values name_id_map and current_block_id in RGMBBase to ReactorMeshParams
  updateReactorMeshParams();

  return std::move(*_build_mesh);
}
