//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactorGeometryMeshBuilderBase.h"
#include "DepletionIDGenerator.h"
#include "MooseMeshUtils.h"
#include "CSGPlane.h"

InputParameters
ReactorGeometryMeshBuilderBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addDeprecatedParam<bool>("show_rgmb_metadata",
                                  "Print out RGMB-related metadata to console output",
                                  "This parameter is deprecated. Please use MeshMetaDataReporter "
                                  "system to print out mesh metadata to JSON output file instead");
  params.addClassDescription("A base class that contains common members and methods for Reactor "
                             "Geometry Mesh Builder mesh generators.");

  // Declare that this generator has a generateData method
  MeshGenerator::setHasGenerateData(params);
  return params;
}

void
ReactorGeometryMeshBuilderBase::addDepletionIDParams(InputParameters & params)
{
  params.addParam<bool>(
      "generate_depletion_id", false, "Determine wheter the depletion ID is assigned.");
  MooseEnum depletion_id_option("assembly assembly_type pin pin_type");
  params.addParam<MooseEnum>("depletion_id_type",
                             depletion_id_option,
                             "Determine level of details in depletion ID assignment.");
  params.addParamNamesToGroup("generate_depletion_id depletion_id_type", "Depletion ID assignment");
}

ReactorGeometryMeshBuilderBase::ReactorGeometryMeshBuilderBase(const InputParameters & parameters)
  : MeshGenerator(parameters)
{
}

void
ReactorGeometryMeshBuilderBase::initializeReactorMeshParams(const std::string reactor_param_name)
{
  _reactor_params = reactor_param_name;

  // Ensure that the user has supplied a valid ReactorMeshParams object
  _reactor_params_mesh = &getMeshByName(reactor_param_name);
  if (*_reactor_params_mesh)
    mooseError("The reactor_params mesh is not of the correct type");

  if (!hasMeshProperty<unsigned int>("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty<std::string>("mesh_geometry", _reactor_params))
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n Please "
               "check that a valid definition and name of ReactorMeshParams has been provided.");

  // Set reactor_params_name metadata for use by future mesh generators
  declareMeshProperty("reactor_params_name", std::string(_reactor_params));

  // Store CSGBase object if we are in CSG only mode
  if (_app.getMeshGeneratorSystem().getCSGOnly())
    _reactor_params_csg = &getCSGBaseByName(_reactor_params);
}

void
ReactorGeometryMeshBuilderBase::freeReactorParamsMesh()
{
  _reactor_params_mesh->reset();
}

void
ReactorGeometryMeshBuilderBase::freeReactorParamsCSG()
{
  _reactor_params_csg->reset();
}

unsigned int
ReactorGeometryMeshBuilderBase::getElemIntegerFromMesh(MeshBase & input_mesh,
                                                       std::string extra_int_name,
                                                       bool should_exist)
{
  if (input_mesh.has_elem_integer(extra_int_name))
    return input_mesh.get_elem_integer_index(extra_int_name);
  else
  {
    if (should_exist)
      mooseError("Expected extruded mesh to have " + extra_int_name + " extra integers");
    else
      return input_mesh.add_elem_integer(extra_int_name);
  }
}

void
ReactorGeometryMeshBuilderBase::updateElementBlockNameId(
    MeshBase & input_mesh,
    Elem * elem,
    std::map<std::string, SubdomainID> & name_id_map,
    std::string elem_block_name,
    SubdomainID & next_free_id)
{
  SubdomainID elem_block_id;
  if (name_id_map.find(elem_block_name) == name_id_map.end())
  {
    // Block name does not exist in mesh yet, assign new block id and name
    elem_block_id = next_free_id++;
    elem->subdomain_id() = elem_block_id;
    input_mesh.subdomain_name(elem_block_id) = elem_block_name;
    name_id_map[elem_block_name] = elem_block_id;
  }
  else
  {
    // Block name exists in mesh, reuse block id
    elem_block_id = name_id_map[elem_block_name];
    elem->subdomain_id() = elem_block_id;
  }
}

void
ReactorGeometryMeshBuilderBase::addDepletionId(MeshBase & input_mesh,
                                               const MooseEnum & option,
                                               const DepletionIDGenerationLevel generation_level,
                                               const bool extrude)
{
  // prepare set of extra elem ids for depletion ID generation
  std::vector<ExtraElementIDName> id_names = {};
  if (extrude)
    id_names.push_back("plane_id");
  if (generation_level == DepletionIDGenerationLevel::Core)
  {
    if (option == "pin")
      id_names.insert(id_names.end(), {"assembly_id", "pin_id"});
    else if (option == "pin_type")
      id_names.insert(id_names.end(), {"assembly_id", "pin_type_id"});
    else if (option == "assembly")
      id_names.push_back("assembly_id");
    else if (option == "assembly_type")
      id_names.push_back("assembly_type_id");
  }
  else if (generation_level == DepletionIDGenerationLevel::Assembly)
  {
    if (option == "pin")
      id_names.push_back("pin_id");
    else if (option == "pin_type")
      id_names.push_back("pin_type_id");
    else
      paramError("depletion_id_type",
                 "'assembly_id' or 'assembly_type_id' is not allowed in depletion ID generation at "
                 "assembly level");
  }
  else if (generation_level == DepletionIDGenerationLevel::Drum)
  {
    if (option == "pin_type")
      id_names.push_back("pin_type_id");
    else
      paramError("depletion_id_type",
                 "Only 'pin_type' is allowed in depletion ID generation at "
                 "drum level");
  }
  else if (generation_level == DepletionIDGenerationLevel::Pin)
    mooseError("Depletion ID generation is not supported at pin level yet in RGMB");
  id_names.push_back("region_id");
  // no block restriction
  std::set<SubdomainID> block_ids = {};
  // create depletion IDs
  // depletion IDs will be assigned in the following order:
  // regions (materials) within pin -> pins in assembly -> assemblies in core -> axial planes
  std::unordered_map<dof_id_type, dof_id_type> depl_ids =
      MooseMeshUtils::getExtraIDUniqueCombinationMap(input_mesh, block_ids, id_names);
  // assign depletion ids to elements
  const auto depl_id_index = input_mesh.add_elem_integer("depletion_id");
  for (Elem * const elem : input_mesh.active_element_ptr_range())
    elem->set_extra_integer(depl_id_index, depl_ids.at(elem->id()));
}

MeshGeneratorName
ReactorGeometryMeshBuilderBase::callExtrusionMeshSubgenerators(
    const MeshGeneratorName input_mesh_name)
{
  std::vector<Real> axial_boundaries = getReactorParam<std::vector<Real>>(RGMB::axial_mesh_sizes);
  const auto top_boundary = getReactorParam<boundary_id_type>(RGMB::top_boundary_id);
  const auto bottom_boundary = getReactorParam<boundary_id_type>(RGMB::bottom_boundary_id);

  {
    auto params = _app.getFactory().getValidParams("AdvancedExtruderGenerator");

    params.set<MeshGeneratorName>("input") = input_mesh_name;
    params.set<Point>("direction") = Point(0, 0, 1);
    params.set<std::vector<unsigned int>>("num_layers") =
        getReactorParam<std::vector<unsigned int>>(RGMB::axial_mesh_intervals);
    params.set<std::vector<Real>>("heights") = axial_boundaries;
    params.set<BoundaryName>("bottom_boundary") = std::to_string(bottom_boundary);
    params.set<BoundaryName>("top_boundary") = std::to_string(top_boundary);
    addMeshSubgenerator("AdvancedExtruderGenerator", name() + "_extruded", params);
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

  const MeshGeneratorName output_mesh_name = name() + "_extrudedIDs";
  {
    auto params = _app.getFactory().getValidParams("PlaneIDMeshGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_change_plane_name";

    std::vector<Real> plane_heights{0};
    for (Real z : axial_boundaries)
      plane_heights.push_back(z + plane_heights.back());

    params.set<std::vector<Real>>("plane_coordinates") = plane_heights;

    std::string plane_id_name = "plane_id";
    params.set<std::string>("id_name") = "plane_id";

    addMeshSubgenerator("PlaneIDMeshGenerator", output_mesh_name, params);
  }

  return output_mesh_name;
}

std::vector<std::reference_wrapper<const CSG::CSGSurface>>
ReactorGeometryMeshBuilderBase::getOuterRadialSurfaces(unsigned int radial_index,
                                                       Real halfpitch,
                                                       CSG::CSGBase & csg_obj)
{
  std::vector<std::reference_wrapper<const CSG::CSGSurface>> duct_surfaces;
  const auto mesh_geometry = getReactorParam<std::string>(RGMB::mesh_geometry);
  auto n_surfaces = mesh_geometry == "Square" ? 4 : 6;

  // Convert halfpitch to radius (distance from vertex to center)
  Real angle_offset_degrees = mesh_geometry == "Square" ? 45 : 30;
  Real angle_offset_radians = angle_offset_degrees * (M_PI / 180.);
  const auto radius = halfpitch / std::cos(angle_offset_radians);

  Real angle_increment_radians = 360. / n_surfaces * (M_PI / 180.);

  for (const auto i : make_range(n_surfaces))
  {
    const auto surf_name =
        name() + "_radial_duct_" + std::to_string(radial_index) + "_surf_" + std::to_string(i);

    // Define 3 points on the surface
    const auto current_angle = i * angle_increment_radians + angle_offset_radians;
    const auto next_angle = (i + 1) * angle_increment_radians + angle_offset_radians;
    libMesh::Point p0(radius * std::cos(current_angle), radius * std::sin(current_angle), 0.);
    libMesh::Point p1(radius * std::cos(next_angle), radius * std::sin(next_angle), 0.);
    libMesh::Point p2 = (p0 + p1) / 2.;
    // Place third point above the two others to form a vertical plane
    p2(2) = angle_offset_degrees;

    std::unique_ptr<CSG::CSGSurface> duct_surf_ptr =
        std::make_unique<CSG::CSGPlane>(surf_name, p0, p1, p2);
    const auto & duct_surf = csg_obj.addSurface(std::move(duct_surf_ptr));
    duct_surfaces.push_back(duct_surf);
  }

  return duct_surfaces;
}

std::vector<std::reference_wrapper<const CSG::CSGSurface>>
ReactorGeometryMeshBuilderBase::getAxialPlaneSurfaces(CSG::CSGBase & csg_obj)
{
  std::vector<std::reference_wrapper<const CSG::CSGSurface>> surfaces_by_axial_region;
  const auto axial_boundaries = getReactorParam<std::vector<Real>>(RGMB::axial_mesh_sizes);
  Real axial_level = 0.;

  // Check if axial planes have been defined in CSGBase based on a surface name we expect
  // to find
  auto axial_surf_name = RGMB::CSG_AXIAL_PLANE_PREFIX + "0";
  const auto has_axial_surfaces = csg_obj.hasSurface(axial_surf_name);

  for (const auto i : make_range(axial_boundaries.size() + 1))
  {
    axial_surf_name = RGMB::CSG_AXIAL_PLANE_PREFIX + std::to_string(i);
    if (has_axial_surfaces)
      // Surface should exist in CSGBase, retrieve from object
      surfaces_by_axial_region.push_back(csg_obj.getSurfaceByName(axial_surf_name));
    else
    {
      // Surface has not been defined, create it and add to CSGBase
      axial_level += (i != 0) ? axial_boundaries[i - 1] : 0.;
      std::unique_ptr<CSG::CSGSurface> plane_surf_ptr =
          std::make_unique<CSG::CSGPlane>(axial_surf_name, 0, 0, 1, axial_level);
      const auto & plane_surf = csg_obj.addSurface(std::move(plane_surf_ptr));
      surfaces_by_axial_region.push_back(plane_surf);
    }
  }

  return surfaces_by_axial_region;
}
