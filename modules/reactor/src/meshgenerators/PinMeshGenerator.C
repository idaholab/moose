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

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "region_ids",
      "IDs for each radial and axial zone for assignment of block id and region_id extra element "
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
  : MeshGenerator(parameters),
    _reactor_params(getParam<MeshGeneratorName>("reactor_params")),
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
  declareMeshProperty("pitch", _pitch);

  // Ensure that the user has supplied the correct info for conformal mesh generation
  if (getMeshByName(_reactor_params) != nullptr)
    mooseError("The reactor_params mesh is not of the correct type");

  if (!hasMeshProperty("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty("mesh_geometry", _reactor_params))
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n Please "
               "check that a valid definition and name of ReactorMeshParams has been provided.");
  else
  {
    _mesh_dimensions = getMeshProperty<int>("mesh_dimensions", _reactor_params);
    _mesh_geometry = getMeshProperty<std::string>("mesh_geometry", _reactor_params);
  }
  if (!_quad_center && _intervals[0] != 1)
    mooseError("The number of mesh intervals in the ring region must be one if "
               "quad_center_elements is not true");
  if (_extrude && _mesh_dimensions != 3)
    mooseError("This is a 2 dimensional mesh, you cannot extrude it. Check your ReactorMeshParams "
               "inputs\n");
  if (_extrude && (!hasMeshProperty("top_boundary_id", _reactor_params) ||
                   !hasMeshProperty("bottom_boundary_id", _reactor_params)))
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

  // Define all id variables used in the pin
  std::vector<unsigned int> ring_intervals;
  std::vector<subdomain_id_type> ring_blk_ids;
  unsigned int background_intervals = 1;
  std::vector<subdomain_id_type> background_ids;
  std::vector<unsigned int> duct_intervals;
  std::vector<subdomain_id_type> duct_ids;

  for (const auto i : make_range(_intervals.size()))
  {
    if (i < _ring_radii.size())
    {
      ring_intervals.push_back(_intervals[i]);
      ring_blk_ids.push_back(_region_ids[0][i]);
    }
    else if (i > _ring_radii.size())
    {
      duct_intervals.push_back(_intervals[i]);
      duct_ids.push_back(_region_ids[0][i]);
    }
    else
    {
      background_intervals = _intervals[i];
      background_ids.push_back(_region_ids[0][i]);
    }
  }
  if (ring_intervals.size() > 0)
  {
    if (ring_intervals.front() != 1)
      ring_blk_ids.insert(ring_blk_ids.begin(), ring_blk_ids.front());
  }
  else if (background_intervals > 1)
  {
    background_ids.insert(background_ids.begin(), background_ids.front());
  }

  if (_mesh_geometry == "Square")
  {
    {
      // Get and assign parameters for the main geometry feature of the Pin
      // which is created with a PolygonConcentricCircleMeshGenerator subgenerator
      auto params = _app.getFactory().getValidParams("PolygonConcentricCircleMeshGenerator");

      params.set<unsigned int>("num_sides") = 4; // Cartesian geometry so pin are given a square box
      params.set<std::vector<unsigned int>>("num_sectors_per_side") =
          std::vector<unsigned int>(4, _num_sectors);
      params.set<bool>("preserve_volumes") = true;
      params.set<bool>("quad_center_elements") = _quad_center;

      if (ring_intervals.size() > 0)
      {
        params.set<std::vector<Real>>("ring_radii") = _ring_radii;
        params.set<std::vector<subdomain_id_type>>("ring_block_ids") = ring_blk_ids;
        params.set<std::vector<unsigned int>>("ring_intervals") = ring_intervals;
      }

      params.set<std::vector<subdomain_id_type>>("background_block_ids") = background_ids;
      params.set<unsigned int>("background_intervals") = background_intervals;

      if (duct_intervals.size() > 0)
      {
        params.set<MooseEnum>("duct_sizes_style") = "apothem";
        params.set<std::vector<Real>>("duct_sizes") = _duct_halfpitch;
        params.set<std::vector<subdomain_id_type>>("duct_block_ids") = duct_ids;
        params.set<std::vector<unsigned int>>("duct_intervals") = duct_intervals;
      }

      params.set<MooseEnum>("polygon_size_style") = "apothem";
      params.set<Real>("polygon_size") = _pitch / 2.0;
      params.set<bool>("flat_side_up") = true;
      params.set<boundary_id_type>("external_boundary_id") = 20000 + _pin_type;
      params.set<boundary_id_type>("interface_boundary_id_shift") =
          30000; // need to shift interface boundaries to avoid clashing
                 // with default IDs for PatternedMeshGenerator

      addMeshSubgenerator("PolygonConcentricCircleMeshGenerator", name() + "_circle", params);
    }

    // Define boundary IDs so that there are unified boundaries for use in patterned MeshGenerator
    // if this pin is to be used in an AssemblyMeshGenerator
    {
      auto params = _app.getFactory().getValidParams("SideSetsFromNormalsGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_circle";
      params.set<std::vector<Point>>("normals") = {
          {1, 0, 0},
          {-1, 0, 0},
          {0, 1, 0},
          {0, -1, 0}}; // normal directions over which to define boundaries
      params.set<bool>("fixed_normal") = true;
      params.set<bool>("replace") = false;
      params.set<std::vector<BoundaryName>>("new_boundary") = {
          "tmp_left", "tmp_right", "tmp_top", "tmp_bottom"};

      _build_mesh = &addMeshSubgenerator("SideSetsFromNormalsGenerator", name() + "_bds", params);
    }
    {
      auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_bds";
      params.set<std::vector<BoundaryName>>("old_boundary") = {
          "tmp_left", "tmp_right", "tmp_top", "tmp_bottom"};
      auto external_boundary_name = "outer_pin_" + std::to_string(_pin_type);
      params.set<std::vector<BoundaryName>>("new_boundary") =
          std::vector<BoundaryName>(4, external_boundary_name);

      _build_mesh = &addMeshSubgenerator("RenameBoundaryGenerator", name() + "_2D", params);
    }
  }
  else
  {
    // Hex geometry
    {
      auto params = _app.getFactory().getValidParams("PolygonConcentricCircleMeshGenerator");
      params.set<unsigned int>("num_sides") = 6;
      params.set<std::vector<unsigned int>>("num_sectors_per_side") =
          std::vector<unsigned int>(6, _num_sectors);
      params.set<bool>("preserve_volumes") = true;
      params.set<bool>("quad_center_elements") = _quad_center;
      params.set<boundary_id_type>("external_boundary_id") =
          20000 +
          _pin_type; // The boundary id for pins as set by the Reactor Geometry Meshing convention
      params.set<std::string>("external_boundary_name") = "outer_pin_" + std::to_string(_pin_type);

      if (ring_intervals.size() > 0)
      {
        params.set<std::vector<Real>>("ring_radii") = _ring_radii;
        params.set<std::vector<subdomain_id_type>>("ring_block_ids") = ring_blk_ids;
        params.set<std::vector<unsigned int>>("ring_intervals") = ring_intervals;
      }

      params.set<std::vector<subdomain_id_type>>("background_block_ids") = background_ids;
      params.set<unsigned int>("background_intervals") = background_intervals;

      if (duct_intervals.size() > 0)
      {
        params.set<MooseEnum>("duct_sizes_style") = "apothem";
        params.set<std::vector<Real>>("duct_sizes") = _duct_halfpitch;
        params.set<std::vector<subdomain_id_type>>("duct_block_ids") = duct_ids;
        params.set<std::vector<unsigned int>>("duct_intervals") = duct_intervals;
      }
      params.set<MooseEnum>("polygon_size_style") = "apothem";
      params.set<Real>("polygon_size") = _pitch / 2.0;

      _build_mesh =
          &addMeshSubgenerator("PolygonConcentricCircleMeshGenerator", name() + "_2D", params);
    }
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
    if (num_sides == 4)
      boundaries_to_delete.push_back(std::to_string(20000 + _pin_type));
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

  // transferring reactor parameters and PolygonConcentricCircleMeshGenerator parameters to pin mesh
  declareMeshProperty("reactor_params_name", std::string(_reactor_params));

  if (_extrude && _mesh_dimensions == 3)
  {
    std::vector<Real> axial_boundaries =
        getMeshProperty<std::vector<Real>>("axial_boundaries", _reactor_params);
    const auto top_boundary = getMeshProperty<boundary_id_type>("top_boundary_id", _reactor_params);
    const auto bottom_boundary =
        getMeshProperty<boundary_id_type>("bottom_boundary_id", _reactor_params);
    {
      declareMeshProperty("extruded", true);
      auto params = _app.getFactory().getValidParams("FancyExtruderGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_del_bds";
      params.set<Point>("direction") = Point(0, 0, 1);
      params.set<std::vector<unsigned int>>("num_layers") =
          getMeshProperty<std::vector<unsigned int>>("axial_mesh_intervals", _reactor_params);
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
}

std::unique_ptr<MeshBase>
PinMeshGenerator::generate()
{
  // This generate() method will be called once the subgenerators that we depend on are
  // called. This is where we set the region ids and element integers.
  if (_extrude)
  {
    // swap the region ids on the subdomain ids to the correct ones
    // for their axial layer
    std::string plane_id_name = "plane_id";
    unsigned int pid_int = (*_build_mesh)->get_elem_integer_index(plane_id_name);
    for (const auto i : make_range(_region_ids[0].size()))
    {
      subdomain_id_type region = _region_ids[0][i];
      for (const auto & elem : as_range((*_build_mesh)->active_subdomain_elements_begin(region),
                                        (*_build_mesh)->active_subdomain_elements_end(region)))
      {
        dof_id_type z_id = elem->get_extra_integer(pid_int);
        subdomain_id_type r_id = _region_ids[std::size_t(z_id)][i];

        // only swap if necessary
        if (r_id != region)
          elem->subdomain_id() = r_id;
      }
    }
  }

  // Add region IDs to the mesh as element integers
  std::string region_id_name = "region_id";
  unsigned int rid;
  if (!(*_build_mesh)->has_elem_integer(region_id_name))
    rid = (*_build_mesh)->add_elem_integer(region_id_name);
  else
    rid = (*_build_mesh)->get_elem_integer_index(region_id_name);

  std::string pin_type_id_name = "pin_type_id";
  unsigned int ptid = 0;
  if (!(*_build_mesh)->has_elem_integer(pin_type_id_name))
    ptid = (*_build_mesh)->add_elem_integer(pin_type_id_name);
  else
    ptid = (*_build_mesh)->get_elem_integer_index(pin_type_id_name);

  // set element names to element IDs to overwrite conflicting assignment from subgenerators
  std::map<subdomain_id_type, std::string> subdomain_name_map;
  for (auto & elem : (*_build_mesh)->active_element_ptr_range())
  {
    elem->set_extra_integer(ptid, _pin_type);
    elem->set_extra_integer(rid, elem->subdomain_id());

    if (subdomain_name_map.find(elem->subdomain_id()) == subdomain_name_map.end())
      subdomain_name_map.insert(std::pair<subdomain_id_type, std::string>(
          elem->subdomain_id(), std::to_string(elem->subdomain_id())));
  }

  (*_build_mesh)->set_subdomain_name_map() = subdomain_name_map;

  return std::move(*_build_mesh);
}
