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
      "ring_radii", "ring_radii>0.0", "Radii of major concentric circles");

  params.addRangeCheckedParam<std::vector<Real>>(
      "duct_halfpitch", "duct_halfpitch>0.0", "Apothem of the ducts");

  params.addRangeCheckedParam<std::vector<unsigned int>>(
      "mesh_intervals",
      std::vector<unsigned int>{1},
      "mesh_intervals>0",
      "The number of meshing intervals for each region starting at the center (length(ring_radii) "
      "+ "
      "length(duct_halfpitch )+ 1)");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "region_ids",
      "IDs for each radial and axial zone for assignment of unique physical characteristics");

  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building");

  params.addParam<bool>(
      "quad_center_elements", true, "Whether the center elements are quad or triangular.");

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
  // Ensure that the user has supplied the correct info for conformal mesh generation

  if (getMeshByName(_reactor_params) != nullptr)
    mooseError("The reactor_params mesh is not of the correct type");
  else
    _procedural_ids = getMeshProperty<bool>("procedural_ids", _reactor_params);

  if (!hasMeshProperty("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty("mesh_geometry", _reactor_params))
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n");
  else
  {
    _mesh_dimensions = getMeshProperty<int>("mesh_dimensions", _reactor_params);
    _mesh_geometry = getMeshProperty<std::string>("mesh_geometry", _reactor_params);
  }
  if (_extrude && _mesh_dimensions != 3)
    mooseError("This is a 2 dimensional mesh, you cannot extrude it. Check your ReactorMeshParams "
               "inputs\n");

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
  else if (!_procedural_ids)
  {
    mooseError("If procedural ID generation is not being used then region IDs must be assigned "
               "with parameter region_ids");
  }
  else
  {
    // procedurally generate region ids, making centermost interval its own region
    // IDs are generated attempting to concatenate the pin type ID and the radial region
    // with the centermost being region 0. If the pin type ID is too large or there
    // are too many radial regions the concatenation will truncate the end of the pin type ID.
    // The region ID is limited by the subdomain_id_type which is uint16.

    int num_rings = _intervals.size();
    if (_intervals[0] > 1)
    {
      // To allow quad and tri center elements the innermost ring must be
      // a unique region
      if (_ring_radii.size() > 0)
      {
        _ring_radii.insert(_ring_radii.begin(), _ring_radii[0] / _intervals[0]);
        _intervals[0] = _intervals[0] - 1;
        _intervals.insert(_intervals.begin(), 1);
      }
      // else the background is the inner most region
      // so interval handling is done during params assignment
      // background center -> generate id but don't break interval yet
      num_rings++;
    }

    // checking to see if the concatenization of pin_type and ring number will fit in a unint16
    // (subdomain_id_type)
    subdomain_id_type ident;
    int digits_r = floor(log10(num_rings)) + 1;
    int digits_id = floor(log10(_pin_type)) + 1;
    if (_pin_type * pow(10, digits_r) + num_rings > UINT16_MAX - 1)
    {
      // too big to concatenate, check if truncated pin_type_id to 5 digits will fit.
      // If not, truncating to 4 digits will always fit.
      if (_pin_type * pow(10, 5 - digits_id) + num_rings > UINT16_MAX - 1)
        ident = _pin_type * pow(10, 4 - digits_id) -
                int(_pin_type * pow(10, 4 - digits_id)) % int(digits_r);
      else
        ident = _pin_type * pow(10, 5 - digits_id) -
                int(_pin_type * pow(10, 5 - digits_id)) % int(digits_r);
    }
    else
      ident = _pin_type * pow(10, digits_r);

    std::vector<subdomain_id_type> regions_0;
    for (subdomain_id_type i = 0; i < num_rings; i++)
    {
      if (!(_intervals[0] > 1 && i == 0))
        // for center background skip the first to assign in params
        regions_0.push_back(ident + i + 1);
    }

    _region_ids.push_back(regions_0);
    if (_mesh_dimensions == 3)
    {
      for (size_t i = 1;
           i < getMeshProperty<std::vector<Real>>("axial_boundaries", _reactor_params).size();
           i++)
      {
        _region_ids.push_back(regions_0);
      }
    }
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

      // Break the mesh intervals and region ids up by construct for
      // PolygonConcentricCircleMeshGenerator
      std::vector<unsigned int> ring_intervals;
      std::vector<subdomain_id_type> ring_ids;
      unsigned int background_intervals = 1;
      std::vector<subdomain_id_type> background_ids;
      std::vector<unsigned int> duct_intervals;
      std::vector<subdomain_id_type> duct_ids;
      for (size_t i = 0; i < _intervals.size(); i++)
      {
        if (i < _ring_radii.size())
        {
          ring_intervals.push_back(_intervals[i]);
          ring_ids.push_back(_region_ids[0][i]);
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
        params.set<std::vector<Real>>("ring_radii") = _ring_radii;
        if (ring_intervals.front() != 1)
          ring_ids.insert(ring_ids.begin(), ring_ids.front());
        params.set<std::vector<subdomain_id_type>>("ring_block_ids") = ring_ids;
        params.set<std::vector<unsigned int>>("ring_intervals") = ring_intervals;
      }
      else if (background_intervals > 1)
      {
        // background center so if procedural add the extra region ID
        if (_procedural_ids)
          background_ids.insert(background_ids.begin(), background_ids.front() - 1);
        // set innermost interval of region to the same ID as the rest of the region
        else
          background_ids.insert(background_ids.begin(), background_ids.front());
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
      params.set<boundary_id_type>("external_boundary_id") = 20000 + _pin_type;
      params.set<boundary_id_type>("interface_boundary_id_shift") =
          30000; // need to shift interface boundaries to avoid clashing
                 // with default IDs for PatternedMeshGenerator
      params.set<std::string>("external_boundary_name") = "outer_pin_" + std::to_string(_pin_type);

      addMeshSubgenerator("PolygonConcentricCircleMeshGenerator", name() + "_circle", params);
    }

    // Rotate pin to be square rather than diamond, like what is generated using
    // ConcentricCircleMeshGenerator
    {
      auto params = _app.getFactory().getValidParams("TransformGenerator");
      params.set<MeshGeneratorName>("input") = name() + "_circle";
      params.set<MooseEnum>("transform") = 4;
      params.set<RealVectorValue>("vector_value") = RealVectorValue(0, 0, 45);

      //_build_mesh = &addMeshSubgenerator("TransformGenerator", name() + "_2D", params);
      addMeshSubgenerator("TransformGenerator", name() + "trans", params);
    }

    // Pass mesh meta-data from subgenerator to this MeshGenerator
    if (hasMeshProperty("pitch_meta", name() + "_circle"))
      declareMeshProperty("pitch_meta", getMeshProperty<Real>("pitch_meta", name() + "_circle"));
    if (hasMeshProperty("background_intervals_meta", name() + "_circle"))
      declareMeshProperty(
          "background_intervals_meta",
          getMeshProperty<unsigned int>("background_intervals_meta", name() + "_circle"));
    if (hasMeshProperty("node_id_background_meta", name() + "_circle"))
      declareMeshProperty(
          "node_id_background_meta",
          getMeshProperty<unsigned int>("node_id_background_meta", name() + "_circle"));
    if (hasMeshProperty("pattern_pitch_meta", name() + "_circle"))
      declareMeshProperty("pattern_pitch_meta",
                          getMeshProperty<Real>("pattern_pitch_meta", name() + "_circle"));
    if (hasMeshProperty("azimuthal_angle_meta", name() + "_circle"))
      declareMeshProperty(
          "azimuthal_angle_meta",
          getMeshProperty<std::vector<Real>>("azimuthal_angle_meta", name() + "_circle"));
    if (hasMeshProperty("num_sectors_per_side_meta", name() + "_circle"))
      declareMeshProperty("num_sectors_per_side_meta",
                          getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta",
                                                                     name() + "_circle"));
    if (hasMeshProperty("max_radius_meta", name() + "_circle"))
      declareMeshProperty("max_radius_meta",
                          getMeshProperty<Real>("max_radius_meta", name() + "_circle"));

    // Change boundary IDs so that there are unified boundaries for use in patterned MeshGenerator
    // if this pin is to be used in an AssemblyMeshGenerator
    {
      auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");

      params.set<MeshGeneratorName>("input") = name() + "trans";
      params.set<std::vector<BoundaryName>>("old_boundary") = {
          "15001",
          "15002",
          "15003",
          "15004"}; // hard coded boundary IDs in patterned mesh generator
      params.set<std::vector<BoundaryName>>("new_boundary") = {"10001", "10002", "10003", "10004"};

      _build_mesh = &addMeshSubgenerator("RenameBoundaryGenerator", name() + "_2D", params);
    }
  }
  else
  {
    // Hex geometry
    {
      auto params = _app.getFactory().getValidParams("PolygonConcentricCircleMeshGenerator");
      params.set<unsigned int>("num_sides") = 6; // Cartesian geometry so pin are given a square box
      params.set<std::vector<unsigned int>>("num_sectors_per_side") =
          std::vector<unsigned int>(6, _num_sectors);
      params.set<bool>("preserve_volumes") = true;
      params.set<bool>("quad_center_elements") = _quad_center;
      params.set<boundary_id_type>("external_boundary_id") =
          20000 +
          _pin_type; // The boundary id for pins as set by the Reactor Geometry Meshing convention
      params.set<std::string>("external_boundary_name") = "outer_pin_" + std::to_string(_pin_type);

      // Break the mesh intervals and region ids up by construct for
      // PolygonConcentricCircleMeshGenerator
      std::vector<unsigned int> ring_intervals;
      std::vector<subdomain_id_type> ring_ids;
      unsigned int background_intervals = 1;
      std::vector<subdomain_id_type> background_ids;
      std::vector<unsigned int> duct_intervals;
      std::vector<subdomain_id_type> duct_ids;
      for (size_t i = 0; i < _intervals.size(); i++)
      {
        if (i < _ring_radii.size())
        {
          ring_intervals.push_back(_intervals[i]);
          ring_ids.push_back(_region_ids[0][i]);
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
        params.set<std::vector<Real>>("ring_radii") = _ring_radii;
        if (ring_intervals.front() != 1)
          ring_ids.insert(ring_ids.begin(), ring_ids.front());

        params.set<std::vector<subdomain_id_type>>("ring_block_ids") = ring_ids;
        params.set<std::vector<unsigned int>>("ring_intervals") = ring_intervals;
      }
      else if (background_intervals > 1)
      {
        if (_procedural_ids)
          background_ids.insert(background_ids.begin(), background_ids.front() - 1);
        else
          background_ids.insert(background_ids.begin(), background_ids.front());
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

    if (hasMeshProperty("pitch_meta", name() + "_2D"))
      declareMeshProperty("pitch_meta", getMeshProperty<Real>("pitch_meta", name() + "_2D"));
    if (hasMeshProperty("background_intervals_meta", name() + "_2D"))
      declareMeshProperty(
          "background_intervals_meta",
          getMeshProperty<unsigned int>("background_intervals_meta", name() + "_2D"));
    if (hasMeshProperty("node_id_background_meta", name() + "_2D"))
      declareMeshProperty("node_id_background_meta",
                          getMeshProperty<unsigned int>("node_id_background_meta", name() + "_2D"));
    if (hasMeshProperty("pattern_pitch_meta", name() + "_2D"))
      declareMeshProperty("pattern_pitch_meta",
                          getMeshProperty<Real>("pattern_pitch_meta", name() + "_2D"));
    if (hasMeshProperty("azimuthal_angle_meta", name() + "_2D"))
      declareMeshProperty(
          "azimuthal_angle_meta",
          getMeshProperty<std::vector<Real>>("azimuthal_angle_meta", name() + "_2D"));
    if (hasMeshProperty("num_sectors_per_side_meta", name() + "_2D"))
      declareMeshProperty(
          "num_sectors_per_side_meta",
          getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta", name() + "_2D"));
    if (hasMeshProperty("max_radius_meta", name() + "_2D"))
      declareMeshProperty("max_radius_meta",
                          getMeshProperty<Real>("max_radius_meta", name() + "_2D"));
  }

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
    {
      declareMeshProperty("extruded", true);
      auto params = _app.getFactory().getValidParams("FancyExtruderGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_2D";
      params.set<Point>("direction") = Point(0, 0, 1);
      params.set<std::vector<unsigned int>>("num_layers") =
          getMeshProperty<std::vector<unsigned int>>("axial_mesh_intervals", _reactor_params);
      params.set<std::vector<Real>>("heights") = axial_boundaries;
      params.set<boundary_id_type>("bottom_boundary") = 202;
      params.set<boundary_id_type>("top_boundary") = 201;
      addMeshSubgenerator("FancyExtruderGenerator", name() + "_extruded", params);
    }

    {
      auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_extruded";
      params.set<std::vector<BoundaryName>>("old_boundary") = {
          "201", "202"}; // hard coded boundary IDs in patterned mesh generator
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
    mesh_name = name() + "_extrudedIDs";
    std::string plane_id_name = "plane_id";
    unsigned int pid_int = (*_build_mesh)->get_elem_integer_index(plane_id_name);
    for (size_t i = 0; i < _region_ids[0].size(); ++i)
    {
      subdomain_id_type region = _region_ids[0][i];
      for (const auto & elem : as_range((*_build_mesh)->active_subdomain_elements_begin(region),
                                        (*_build_mesh)->active_subdomain_elements_end(region)))
      {
        dof_id_type z_id = elem->get_extra_integer(pid_int);
        subdomain_id_type r_id = _region_ids[size_t(z_id)][i];

        // only swap if necessary
        if (r_id != region)
          elem->subdomain_id() = r_id;
      }
    }
  }
  else
    mesh_name = name() + "_2D";

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
