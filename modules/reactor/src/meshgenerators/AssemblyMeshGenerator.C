//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AssemblyMeshGenerator.h"

#include "MooseApp.h"
#include "Factory.h"
#include "libmesh/elem.h"
#include "MooseMeshUtils.h"

registerMooseObject("ReactorApp", AssemblyMeshGenerator);

InputParameters
AssemblyMeshGenerator::validParams()
{
  auto params = MeshGenerator::validParams();

  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs", "The PinMeshGenerators that form the components of the assembly.");

  params.addRequiredParam<subdomain_id_type>("assembly_type",
                                             "The integer ID for this assembly type definition");

  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "pattern",
      "A double-indexed array starting with the upper-left corner where the index"
      "represents the layout of input pins in the assembly lattice.");

  params.addRangeCheckedParam<std::vector<Real>>(
      "duct_halfpitch",
      "duct_halfpitch>0.0",
      "Distance(s) from center to duct(s) inner boundaries.");

  params.addRangeCheckedParam<unsigned int>("background_intervals",
                                            "background_intervals>0",
                                            "Radial intervals in the assembly peripheral region.");

  params.addRangeCheckedParam<std::vector<unsigned int>>(
      "duct_intervals", "duct_intervals>0", "Number of meshing intervals in each enclosing duct.");

  params.addParam<std::vector<subdomain_id_type>>(
      "background_region_id",
      "The region id for the background area between the pins and the ducts to set block ID and "
      "region_id extra-element integer");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "duct_region_ids",
      "The region id for the ducts from innermost to outermost, to set block ID and region_id "
      "extra-element integer.");

  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building in the Reactor workflow");
  params.addParamNamesToGroup("background_region_id duct_region_ids assembly_type", "ID assigment");
  params.addParamNamesToGroup("background_intervals background_region_id",
                              "Background specifications");
  params.addParamNamesToGroup("duct_intervals duct_region_ids duct_halfpitch",
                              "Duct specifications");

  params.addClassDescription("This AssemblyMeshGenerator object is designed to generate "
                             "assembly-like structures, with IDs, from a reactor geometry. "
                             "The assembly-like structures must consist of a full pattern of equal "
                             "sized pins from PinMeshGenerator"
                             "A hexagonal assembly will be placed inside of a bounding hexagon "
                             "consisting of a background region and, optionally,"
                             " duct regions.");

  return params;
}

AssemblyMeshGenerator::AssemblyMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _inputs(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _assembly_type(getParam<subdomain_id_type>("assembly_type")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _duct_sizes(isParamValid("duct_halfpitch") ? getParam<std::vector<Real>>("duct_halfpitch")
                                               : std::vector<Real>()),
    _background_intervals(
        isParamValid("background_intervals") ? getParam<unsigned int>("background_intervals") : 0),
    _duct_intervals(isParamValid("duct_intervals")
                        ? getParam<std::vector<unsigned int>>("duct_intervals")
                        : std::vector<unsigned int>()),
    _background_region_id(isParamValid("background_region_id")
                              ? getParam<std::vector<subdomain_id_type>>("background_region_id")
                              : std::vector<subdomain_id_type>()),
    _duct_region_ids(isParamValid("duct_region_ids")
                         ? getParam<std::vector<std::vector<subdomain_id_type>>>("duct_region_ids")
                         : std::vector<std::vector<subdomain_id_type>>()),
    _extrude(getParam<bool>("extrude"))
{
  /* Below, we get the ReactorMeshParams info from one of the pin inputs.
     The reactor params gives the axial info if we are to extrude as well as the assembly pitch
     that the pin pattern is to be checked against.
     We then create the PatternedMesh subgenerators and pass the parameter info needed for the
     inputs. Finally, if the mesh is to be extruded we create the FancyExtruderGenerator
     subgenerator and the PlaneIDMeshGenerator subgenerator to handle this.
  */
  MeshGeneratorName _reactor_params =
      MeshGeneratorName(getMeshProperty<std::string>("reactor_params_name", _inputs[0]));

  // Ensure that the user has supplied the correct info for conformal mesh generation
  if (!hasMeshProperty("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty("mesh_geometry", _reactor_params))
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n Please "
               "check that a valid definition and name of ReactorMeshParams has been provided to "
               "the input PinMeshGenerators.");
  else
  {
    _geom_type = getMeshProperty<std::string>("mesh_geometry", _reactor_params);
    _mesh_dimensions = getMeshProperty<int>("mesh_dimensions", _reactor_params);
  }
  if (_extrude && _mesh_dimensions != 3)
    paramError("extrude",
               "This is a 2 dimensional mesh, you cannot extrude it. Check you ReactorMeshParams "
               "inputs\n");
  if (_extrude && (!hasMeshProperty("top_boundary_id", _reactor_params) ||
                   !hasMeshProperty("bottom_boundary_id", _reactor_params)))
    mooseError("Both top_boundary_id and bottom_boundary_id must be provided in ReactorMeshParams "
               "if using extruded geometry");

  Real base_pitch = 0.0;
  for (const auto i : make_range(_inputs.size()))
  {
    auto pin = _inputs[i];
    if (i == 0)
      base_pitch = getMeshProperty<Real>("pitch", pin);
    else
    {
      auto pitch = getMeshProperty<Real>("pitch", pin);
      if (!MooseUtils::absoluteFuzzyEqual(pitch, base_pitch))
        mooseError("All pins within an assembly must have the same pitch");
    }
    if (getMeshProperty<bool>("extruded", pin))
      mooseError("Pins that have already been extruded cannot be used in AssemblyMeshGenerator "
                 "definition.\n");
  }
  auto assembly_pitch = getMeshProperty<Real>("assembly_pitch", _reactor_params);
  if (_geom_type == "Square")
  {
    if (_duct_sizes.size() != 0 || _duct_intervals.size() != 0 || _background_intervals != 0)
      mooseError("Ducts and background regions are not currently supported for square assemblies");
    if ((!MooseUtils::absoluteFuzzyEqual(base_pitch * _pattern.size(), assembly_pitch)) ||
        (!MooseUtils::absoluteFuzzyEqual(base_pitch * _pattern[0].size(), assembly_pitch)))
      mooseError("Assembly pitch must be equal to lattice dimension times pin pitch for Cartesian "
                 "assemblies");
  }

  if (_geom_type == "Hex")
  {
    if ((_background_region_id.size() == 0) || _background_intervals == 0)
      mooseError("Hexagonal assemblies must have a background region defined");
    if (assembly_pitch < _pattern.size() * base_pitch)
      mooseError(
          "Assembly pitch must be larger than the number of assembly rows times the pin pitch");
  }

  if (_duct_sizes.size() != _duct_intervals.size())
    mooseError("If ducts are defined then \"duct_intervals\" and \"duct_region_ids\" must also be "
               "defined and of equal size.");

  if (_duct_sizes.size() != 0)
  {
    if (_duct_region_ids.size() == 0)
      paramError("duct_halfpitch",
                 "If ducts are defined, then \"duct_intervals\" and \"duct_region_ids\" "
                 "must also be defined and of equal size.");
    else if (_duct_region_ids[0].size() != _duct_sizes.size())
      paramError("duct_halfpitch",
                 "If ducts are defined, then \"duct_intervals\" and \"duct_region_ids\" "
                 "must also be defined and of equal size.");
  }

  _assembly_boundary_id = 2000 + _assembly_type;
  _assembly_boundary_name = "outer_assembly_" + std::to_string(_assembly_type);

  if (_geom_type == "Square")
  {
    {
      auto params = _app.getFactory().getValidParams("CartesianIDPatternedMeshGenerator");

      params.set<std::string>("id_name") = "pin_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      params.set<BoundaryName>("top_boundary") = "10000";
      params.set<BoundaryName>("left_boundary") = "10000";
      params.set<BoundaryName>("bottom_boundary") = "10000";
      params.set<BoundaryName>("right_boundary") = "10000";

      _build_mesh =
          &addMeshSubgenerator("CartesianIDPatternedMeshGenerator", name() + "_lattice", params);
    }
    {
      auto params = _app.getFactory().getValidParams("SideSetsFromNormalsGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_lattice";
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
      params.set<std::vector<BoundaryName>>("new_boundary") =
          std::vector<BoundaryName>(4, _assembly_boundary_name);

      _build_mesh = &addMeshSubgenerator("RenameBoundaryGenerator", name() + "_pattern", params);
    }
    //***Add assembly duct around PatternedMesh
  }
  else
  {
    // Hex Geometry
    {
      auto params = _app.getFactory().getValidParams("HexIDPatternedMeshGenerator");

      params.set<std::string>("id_name") = "pin_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      params.set<Real>("hexagon_size") =
          getMeshProperty<Real>("assembly_pitch", _reactor_params) / 2.0;
      params.set<MooseEnum>("hexagon_size_style") = "apothem";
      params.set<unsigned int>("background_intervals") = _background_intervals;
      params.set<subdomain_id_type>("background_block_id") =
          UINT16_MAX - 1; // place holder subdomain id to indicate background region
      _peripheral_regions++;

      if (_duct_sizes.size() > 0)
      {
        std::vector<subdomain_id_type> tmp_duct_regions;
        for (std::size_t duct_it = 0; duct_it < _duct_region_ids[0].size(); ++duct_it)
        {
          // loop through ducts and set their subdomain id as UINT16_MAX-(i+1)
          tmp_duct_regions.push_back(UINT16_MAX - 1 - _peripheral_regions);
          _peripheral_regions++;
        }

        params.set<std::vector<Real>>("duct_sizes") = _duct_sizes;
        params.set<std::vector<subdomain_id_type>>("duct_block_ids") = tmp_duct_regions;
        params.set<std::vector<unsigned int>>("duct_intervals") = _duct_intervals;
      }

      params.set<boundary_id_type>("external_boundary_id") = _assembly_boundary_id;
      params.set<std::string>("external_boundary_name") = _assembly_boundary_name;

      _build_mesh =
          &addMeshSubgenerator("HexIDPatternedMeshGenerator", name() + "_pattern", params);

      if (hasMeshProperty("pitch_meta", name() + "_pattern"))
        declareMeshProperty("pitch_meta", getMeshProperty<Real>("pitch_meta", name() + "_pattern"));
      if (hasMeshProperty("background_intervals_meta", name() + "_pattern"))
        declareMeshProperty(
            "background_intervals_meta",
            getMeshProperty<unsigned int>("background_intervals_meta", name() + "_pattern"));
      if (hasMeshProperty("node_id_background_meta", name() + "_pattern"))
        declareMeshProperty(
            "node_id_background_meta",
            getMeshProperty<unsigned int>("node_id_background_meta", name() + "_pattern"));
      if (hasMeshProperty("pattern_pitch_meta", name() + "_pattern"))
        declareMeshProperty("pattern_pitch_meta",
                            getMeshProperty<Real>("assembly_pitch", _reactor_params));
      if (hasMeshProperty("azimuthal_angle_meta", name() + "_pattern"))
        declareMeshProperty(
            "azimuthal_angle_meta",
            getMeshProperty<std::vector<Real>>("azimuthal_angle_meta", name() + "_pattern"));
      if (hasMeshProperty("num_sectors_per_side_meta", name() + "_pattern"))
        declareMeshProperty("num_sectors_per_side_meta",
                            getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta",
                                                                       name() + "_pattern"));
      if (hasMeshProperty("max_radius_meta", name() + "_pattern"))
        declareMeshProperty("max_radius_meta",
                            getMeshProperty<Real>("max_radius_meta", name() + "_pattern"));
      if (hasMeshProperty("is_control_drum_meta", name() + "_pattern"))
        declareMeshProperty("is_control_drum_meta",
                            getMeshProperty<bool>("is_control_drum_meta", name() + "_pattern"));
      if (hasMeshProperty("control_drum_positions", name() + "_pattern"))
        declareMeshProperty(
            "control_drum_positions",
            getMeshProperty<std::vector<Point>>("control_drum_positions", name() + "_pattern"));
      if (hasMeshProperty("control_drum_angles", name() + "_pattern"))
        declareMeshProperty(
            "control_drum_angles",
            getMeshProperty<std::vector<Real>>("control_drum_angles", name() + "_pattern"));
      if (hasMeshProperty("control_drums_azimuthal_meta", name() + "_pattern"))
        declareMeshProperty("control_drums_azimuthal_meta",
                            getMeshProperty<std::vector<std::vector<Real>>>(
                                "control_drums_azimuthal_meta", name() + "_pattern"));
      if (hasMeshProperty("position_file_name", name() + "_pattern"))
        declareMeshProperty(
            "position_file_name",
            getMeshProperty<std::string>("position_file_name", name() + "_pattern"));

      declareMeshProperty("background_region_ids", _background_region_id);
      declareMeshProperty("duct_region_ids", _duct_region_ids);
    }
  }
  // transferring reactor parameters to assembly mesh
  declareMeshProperty("reactor_params_name", std::string(_reactor_params));

  for (auto pinMG : _inputs)
  {
    std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> pin_map =
        getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>>(
            "pin_region_ids", pinMG);
    _id_map.insert(std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
        pin_map.begin()->first, pin_map.begin()->second));
  }
  declareMeshProperty("pin_id_map", _id_map);
  declareMeshProperty("assembly_type_id", _assembly_type);
  declareMeshProperty("assembly_pitch", getMeshProperty<Real>("assembly_pitch", _reactor_params));

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

      params.set<MeshGeneratorName>("input") = name() + "_pattern";
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
AssemblyMeshGenerator::generate()
{
  // This generate() method will be called once the subgenerators that we depend on are
  // called. This is where we set the region ids and element integers.

  std::string plane_id_name = "plane_id";
  std::string region_id_name = "region_id";
  std::string pin_type_id_name = "pin_type_id";

  if (!(*_build_mesh)->has_elem_integer(region_id_name) ||
      !(*_build_mesh)->has_elem_integer(pin_type_id_name))
    mooseError("Expected mesh inputs to have extra integer ids");

  unsigned int ptid_int = (*_build_mesh)->get_elem_integer_index(pin_type_id_name);
  unsigned int rid_int = (*_build_mesh)->get_elem_integer_index(region_id_name);

  // go through the background and duct regions and assign their elements
  // pin_type id placeholders and region id place holders
  for (const auto i : make_range(_peripheral_regions))
  {
    subdomain_id_type region = UINT16_MAX - (i + 1);

    for (const auto & elem : as_range((*_build_mesh)->active_subdomain_elements_begin(region),
                                      (*_build_mesh)->active_subdomain_elements_end(region)))
    {
      elem->set_extra_integer(ptid_int, region); // identifier for element not in a pin
      elem->set_extra_integer(rid_int, region);
    }
  }

  if (_extrude)
  {
    // swap the region ids on the subdomain ids to the correct ones
    // for their axial layer
    if (!(*_build_mesh)->has_elem_integer(plane_id_name))
      mooseError("Expected extruded mesh to have plane IDs");

    unsigned int pid_int = (*_build_mesh)->get_elem_integer_index(plane_id_name);

    for (auto & elem : (*_build_mesh)->active_element_ptr_range())
    {
      dof_id_type z_id = elem->get_extra_integer(pid_int);
      dof_id_type pt_id = elem->get_extra_integer(ptid_int);
      subdomain_id_type r_id = elem->subdomain_id();

      // Going through the elements in the mesh checking pin_type_ids to assign their axial
      // region ids
      if (_id_map.find(pt_id) == _id_map.end())
      {
        // region isn't in a pin so it must be a peripheral (background or duct) region of the
        // assembly
        unsigned int peripheral_index = (UINT16_MAX - 1) - r_id;
        if (peripheral_index == 0)
          // background region element
          elem->subdomain_id() = _background_region_id[z_id];
        else
          // duct region element
          elem->subdomain_id() = _duct_region_ids[z_id][peripheral_index - 1];
      }
      else
      {
        // region is in a pin so grab the different axial region ids and swap them
        // since during extrusion all regions are given the same ID as the 2D layer
        for (const auto i : make_range((_id_map.at(pt_id))[0].size()))
        {
          // swap subdomain region ids if they are different
          if (r_id == _id_map.at(pt_id)[0][i] &&
              _id_map.at(pt_id)[0][i] != _id_map.at(pt_id)[z_id][i])
            elem->subdomain_id() = _id_map.at(pt_id)[z_id][i];
        }
      }
    }
  }
  else
  {
    for (const auto i : make_range(_peripheral_regions))
    {
      subdomain_id_type region = UINT16_MAX - (i + 1);

      for (const auto & elem : as_range((*_build_mesh)->active_subdomain_elements_begin(region),
                                        (*_build_mesh)->active_subdomain_elements_end(region)))
      {

        if (i == 0)
          // background region element
          elem->subdomain_id() = _background_region_id[0];
        else
          // duct region element
          elem->subdomain_id() = _duct_region_ids[0][i - 1];
      }
    }
  }

  std::string assembly_type_id_name = "assembly_type_id";
  unsigned int atid = 0;
  if (!(*_build_mesh)->has_elem_integer(assembly_type_id_name))
    atid = (*_build_mesh)->add_elem_integer(assembly_type_id_name);
  else
    atid = (*_build_mesh)->get_elem_integer_index(assembly_type_id_name);

  std::map<subdomain_id_type, std::string> subdomain_name_map;
  for (auto & elem : (*_build_mesh)->active_element_ptr_range())
  {
    elem->set_extra_integer(atid, _assembly_type);
    elem->set_extra_integer(rid_int, elem->subdomain_id());

    if (subdomain_name_map.find(elem->subdomain_id()) == subdomain_name_map.end())
      subdomain_name_map.insert(std::pair<subdomain_id_type, std::string>(
          elem->subdomain_id(), std::to_string(elem->subdomain_id())));
  }

  (*_build_mesh)->set_subdomain_name_map() = subdomain_name_map;

  return std::move(*_build_mesh);
}
