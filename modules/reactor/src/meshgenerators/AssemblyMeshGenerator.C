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
  auto params = ReactorGeometryMeshBuilderBase::validParams();

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
      "The region id for the background area between the pins and the ducts to set region_id "
      "extra-element integer");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "duct_region_ids",
      "The region id for the ducts from innermost to outermost, to set region_id "
      "extra-element integer.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "background_block_name", "The block names for the assembly background regions");

  params.addParam<std::vector<std::vector<std::string>>>(
      "duct_block_names",
      "The block names for the assembly duct regions from innermost to outermost");

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
  : ReactorGeometryMeshBuilderBase(parameters),
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
  // Initialize name_id_map and current_block_id from ReactorMeshParams object stored
  // in pin input
  MeshGeneratorName reactor_params =
      MeshGeneratorName(getMeshProperty<std::string>("reactor_params_name", _inputs[0]));
  initializeReactorMeshParams(reactor_params);

  _geom_type = getReactorParam<std::string>("mesh_geometry");
  _mesh_dimensions = getReactorParam<int>("mesh_dimensions");

  if (_extrude && _mesh_dimensions != 3)
    paramError("extrude",
               "This is a 2 dimensional mesh, you cannot extrude it. Check you ReactorMeshParams "
               "inputs\n");
  if (_extrude && (!hasReactorParam("top_boundary_id") || !hasReactorParam("bottom_boundary_id")))
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
  auto assembly_pitch = getReactorParam<Real>("assembly_pitch");
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

  // Check whether block names are defined properly
  if (isParamValid("background_block_name"))
  {
    _has_background_block_name = true;
    _background_block_name = getParam<std::vector<std::string>>("background_block_name");
    if (_background_region_id.size() != _background_block_name.size())
      mooseError("The size of background_block_name must match the size of background_region_id");
  }
  else
    _has_background_block_name = false;

  if (isParamValid("duct_block_names"))
  {
    _has_duct_block_names = true;
    _duct_block_names = getParam<std::vector<std::vector<std::string>>>("duct_block_names");
    if (_duct_region_ids.size() != _duct_block_names.size())
      mooseError("The size of duct_block_names must match the size of duct_region_ids");
    for (const auto i : make_range(_duct_region_ids.size()))
      if (_duct_region_ids[i].size() != _duct_block_names[i].size())
        mooseError("The size of duct_block_names must match the size of duct_region_ids");
  }
  else
    _has_duct_block_names = false;

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

      addMeshSubgenerator("CartesianIDPatternedMeshGenerator", name() + "_lattice", params);
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

      addMeshSubgenerator("SideSetsFromNormalsGenerator", name() + "_bds", params);
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
      params.set<Real>("hexagon_size") = getReactorParam<Real>("assembly_pitch") / 2.0;
      params.set<MooseEnum>("hexagon_size_style") = "apothem";
      params.set<unsigned int>("background_intervals") = _background_intervals;

      const auto background_region_id = _background_region_id[0];
      const auto background_block_name =
          (_has_background_block_name ? _background_block_name[0]
                                      : _block_name_prefix + std::to_string(background_region_id));
      const auto background_block_id = getBlockId(background_block_name, background_region_id);
      params.set<subdomain_id_type>("background_block_id") = background_block_id;
      params.set<SubdomainName>("background_block_name") = background_block_name;
      _peripheral_region_ids.push_back(background_region_id);

      if (_duct_sizes.size() > 0)
      {
        std::vector<subdomain_id_type> duct_block_ids;
        std::vector<SubdomainName> duct_block_names;
        for (std::size_t duct_it = 0; duct_it < _duct_region_ids[0].size(); ++duct_it)
        {
          const auto duct_region_id = _duct_region_ids[0][duct_it];
          const auto duct_block_name =
              (_has_duct_block_names ? _duct_block_names[0][duct_it]
                                     : _block_name_prefix + std::to_string(duct_region_id));
          const auto duct_block_id = getBlockId(duct_block_name, duct_region_id);
          duct_block_ids.push_back(duct_block_id);
          duct_block_names.push_back(duct_block_name);
          _peripheral_region_ids.push_back(duct_region_id);
        }

        params.set<std::vector<Real>>("duct_sizes") = _duct_sizes;
        params.set<std::vector<subdomain_id_type>>("duct_block_ids") = duct_block_ids;
        params.set<std::vector<SubdomainName>>("duct_block_names") = duct_block_names;
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
        declareMeshProperty("pattern_pitch_meta", getReactorParam<Real>("assembly_pitch"));
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
      declareMeshProperty("background_block_names", _background_block_name);
      declareMeshProperty("duct_block_names", _duct_block_names);
    }
  }

  for (auto pinMG : _inputs)
  {
    std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> region_id_map =
        getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>>(
            "pin_region_ids", pinMG);
    _pin_region_id_map.insert(
        std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
            region_id_map.begin()->first, region_id_map.begin()->second));
    std::map<subdomain_id_type, std::vector<std::vector<std::string>>> block_name_map =
        getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<std::string>>>>(
            "pin_block_names", pinMG);
    _pin_block_name_map.insert(std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(
        block_name_map.begin()->first, block_name_map.begin()->second));
  }
  declareMeshProperty("pin_region_id_map", _pin_region_id_map);
  declareMeshProperty("pin_block_name_map", _pin_block_name_map);
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

  // Save updates to name id map to ReactorMeshParams object
  updateReactorMeshParams();
}

std::unique_ptr<MeshBase>
AssemblyMeshGenerator::generate()
{
  // Re-initialize name_id_map and current_block_id from ReactorMeshParams object
  // in case variables have been modified between constructor and this method
  initializeReactorMeshParams();

  // This generate() method will be called once the subgenerators that we depend on are
  // called. This is where we set all element integers.

  // Define all extra element names and integers
  std::string plane_id_name = "plane_id";
  std::string region_id_name = "region_id";
  std::string pin_type_id_name = "pin_type_id";
  std::string assembly_type_id_name = "assembly_type_id";

  if (!(*_build_mesh)->has_elem_integer(region_id_name) ||
      !(*_build_mesh)->has_elem_integer(pin_type_id_name))
    mooseError("Expected mesh inputs to have region_id and pin_type_id extra integer ids");

  unsigned int ptid_int = (*_build_mesh)->get_elem_integer_index(pin_type_id_name);
  unsigned int rid_int = (*_build_mesh)->get_elem_integer_index(region_id_name);

  unsigned int atid_int, pid_int = 0;
  if (!(*_build_mesh)->has_elem_integer(assembly_type_id_name))
    atid_int = (*_build_mesh)->add_elem_integer(assembly_type_id_name);
  else
    atid_int = (*_build_mesh)->get_elem_integer_index(assembly_type_id_name);

  if (_extrude)
  {
    if (!(*_build_mesh)->has_elem_integer(plane_id_name))
      mooseError("Expected extruded mesh to have plane_id extra integers");
    pid_int = (*_build_mesh)->get_elem_integer_index(plane_id_name);
  }

  // Get region id and block information of elements defined by RGMB so far
  auto name_id_map =
      getReactorParam<std::map<std::string, std::pair<subdomain_id_type, dof_id_type>>>(
          "name_id_map");

  for (auto & elem : (*_build_mesh)->active_element_ptr_range())
  {
    elem->set_extra_integer(atid_int, _assembly_type);
    const dof_id_type pt_id = elem->get_extra_integer(ptid_int);
    const dof_id_type z_id = _extrude ? elem->get_extra_integer(pid_int) : 0;

    if (_pin_region_id_map.find(pt_id) != _pin_region_id_map.end())
    {
      // Pin type element, swap subdomains if necessary
      const dof_id_type base_rid = elem->get_extra_integer(rid_int);
      const auto radial_idx = std::find(_pin_region_id_map[pt_id][0].begin(),
                                        _pin_region_id_map[pt_id][0].end(),
                                        base_rid) -
                              _pin_region_id_map[pt_id][0].begin();
      const auto elem_rid = _pin_region_id_map[pt_id][z_id][radial_idx];

      // swap region ids if they are different
      if (elem_rid != base_rid)
      {
        elem->set_extra_integer(rid_int, elem_rid);
        bool has_block_names = !_pin_block_name_map[pt_id].empty();
        auto elem_block_name = (has_block_names ? _pin_block_name_map[pt_id][z_id][radial_idx]
                                                : _block_name_prefix + std::to_string(elem_rid));
        if (elem->type() == TRI3 || elem->type() == PRISM6)
          elem_block_name += "_TRI";
        const auto elem_block_id = getBlockId(elem_block_name, elem_rid);
        elem->subdomain_id() = elem_block_id;
        (*_build_mesh)->subdomain_name(elem_block_id) = elem_block_name;
      }
    }
    else
    {
      // Assembly peripheral element (background / duct), swap subdomains if
      // necessary and set pin type id to UINT16_MAX - 1 - peripheral index
      const auto base_block_id = elem->subdomain_id();
      const auto base_block_name = (*_build_mesh)->subdomain_name(base_block_id);
      subdomain_id_type base_rid = (name_id_map[base_block_name]).second;
      const auto peripheral_idx =
          std::find(_peripheral_region_ids.begin(), _peripheral_region_ids.end(), base_rid) -
          _peripheral_region_ids.begin();
      bool is_background_region = peripheral_idx == 0;

      subdomain_id_type pin_type = UINT16_MAX - 1 - peripheral_idx;
      elem->set_extra_integer(ptid_int, pin_type);

      const auto elem_rid = (is_background_region ? _background_region_id[z_id]
                                                  : _duct_region_ids[z_id][peripheral_idx - 1]);
      elem->set_extra_integer(rid_int, elem_rid);

      // swap region ids if they are different
      if (elem_rid != base_rid)
      {
        SubdomainName elem_block_name;
        if (is_background_region)
          elem_block_name =
              (_has_background_block_name ? _background_block_name[z_id]
                                          : _block_name_prefix + std::to_string(elem_rid));
        else
          elem_block_name = (_has_duct_block_names ? _duct_block_names[z_id][peripheral_idx - 1]
                                                   : _block_name_prefix + std::to_string(elem_rid));

        if (elem->type() == TRI3 || elem->type() == PRISM6)
          elem_block_name += "_TRI";
        const auto elem_block_id = getBlockId(elem_block_name, elem_rid);
        elem->subdomain_id() = elem_block_id;
        (*_build_mesh)->subdomain_name(elem_block_id) = elem_block_name;
      }
    }
  }

  // Update values of name_id_map and current_block_id in RGMBBase to ReactorMeshParams
  updateReactorMeshParams();

  return std::move(*_build_mesh);
}
