//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CavityComponent.h"
#include "RotationMatrix.h"

registerMooseAction("MooseApp", CavityComponent, "add_mesh_generator");
// CavityComponent is an example of ComponentPhysicsInterface
registerMooseAction("MooseApp", CavityComponent, "init_component_physics");
// CavityComponent is an example of ComponentMaterialPropertyInterface
registerMooseAction("MooseApp", CavityComponent, "add_material");
// CavityComponent is an example of ComponentInitialConditionInterface
registerMooseAction("MooseApp", CavityComponent, "check_integrity");
registerActionComponent("MooseApp", CavityComponent);

InputParameters
CavityComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += ComponentPhysicsInterface::validParams();
  params += ComponentMaterialPropertyInterface::validParams();
  params += ComponentInitialConditionInterface::validParams();
  params += ComponentBoundaryConditionInterface::validParams();
  params.addClassDescription(
      "Cavity component meshing around selected components. The mesh will be formed around the component mesh and thus the cavity surface mesh must be larger than the component. If these enclosed components are connected to "
      "other components inside the enclosure, the cavity mesh will be trimmed to avoid overlapping with these other components");
  MooseEnum dims("0 1 2 3");
  params.addRequiredParam<MooseEnum>("dimension",
                                     dims,
                                     "Dimension of the cavity. 2 for a 2D cavity, and 3 for "
                                     "a 3D cavity");
  params.addParam<SubdomainName>("block", "Block name for the cavity");

  // Geometry
  params.addRequiredParam<MeshGeneratorName>(
      "enclosing_surface_mesh_mg",
      "Mesh generator providing the enclosing surface mesh. Should provide a line mesh in 2D, a "
      "surface mesh in 3D");
  params.addParam<std::vector<ComponentName>>(
      "enclosed_components",
      "Components to enclose with the surface mesh. Their outer boundary will be enclosed");
  params.addParam<Point>("recenter_surface_mesh_to",
                         Point(0, 0, 0),
                         "Whether to center the enclosing surface mesh around "
                         "this position before meshing the cavity. The center of the enclosing "
                         "mesh is computed from its bounding box, not its centroid");

  // Discretization
  params.addRangeCheckedParam<Real>("target_element_volume",
                                    0,
                                    "target_element_volume>=0",
                                    "Target element volume (area in 2D). 0 for no target");

  params.addParamNamesToGroup("target_element_volume", "Discretization");
  params.addParamNamesToGroup("enclosing_surface_mesh_mg recenter_surface_mesh_to",
                              "Enclosing mesh geometry");

  return params;
}

CavityComponent::CavityComponent(const InputParameters & params)
  : ActionComponent(params),
    ComponentPhysicsInterface(params),
    ComponentMaterialPropertyInterface(params),
    ComponentInitialConditionInterface(params),
    ComponentBoundaryConditionInterface(params),
    _target_elem_volume(getParam<Real>("target_element_volume")),
    _center_surface_mesh(getParam<Point>("recenter_surface_mesh_to")),
    _surface_mesh_mg(getParam<MeshGeneratorName>("enclosing_surface_mesh_mg")),
    _components_to_enclose(getParam<std::vector<ComponentName>>("enclosed_components"))
{
  _dimension = getParam<MooseEnum>("dimension");
  // The other component interfaces add their required task
  addRequiredTask("add_mesh_generator");

  if (_dimension < 2)
    paramError("dimension", "Cavity not implemented for 0D and 1D");
}

void
CavityComponent::addMeshGenerators()
{
  // Gather the mesh generators creating the component meshes. Note that we should not stitch to
  // those as the components could be part of a bigger group of components.
  std::vector<MeshGeneratorName> holes_mgs;
  std::vector<BoundaryName> holes_boundaries;
  for (const auto & comp_name : _components_to_enclose)
  {
    const auto & comp = _awh.getAction<ActionComponent>(comp_name);
    // NOTE: this introduces an ordering dependency in the execution of "addMeshGenerator"
    holes_mgs.push_back(comp.meshGeneratorNames().back());
    // use the same hole boundary name for components that will be stitched at the same time
    holes_boundaries.push_back("hole_bdy_" + comp.getCurrentTopLevelMeshGeneratorName());
  }

  // Displace the surface mesh if requested
  MeshGeneratorName enclosure_surface_mg = _surface_mesh_mg;
  if (isParamSetByUser("recenter_surface_mesh_to"))
  {
    // Re-center the mesh to the origin
    const auto translation_mg_origin = _surface_mesh_mg + "_translated_to_origin";
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _surface_mesh_mg;
    params.set<MooseEnum>("transform") = "TRANSLATE_CENTER_ORIGIN";
    params.set<bool>("output") = _verbose;
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "TransformGenerator", translation_mg_origin, params);
    _mg_names.push_back(translation_mg_origin);

    // Re-center to the user-input coordinates
    const auto translation_mg = _surface_mesh_mg + "_translated";
    params.set<MeshGeneratorName>("input") = translation_mg_origin;
    params.set<MooseEnum>("transform") = "TRANSLATE";
    params.set<RealVectorValue>("vector_value") = _center_surface_mesh;
    params.set<bool>("output") = _verbose;
    _app.getMeshGeneratorSystem().addMeshGenerator("TransformGenerator", translation_mg, params);
    _mg_names.push_back(translation_mg);
    enclosure_surface_mg = translation_mg;
  }

  // Create the cavity mesh using a general meshes (for now, Delaunay triangulation or
  // tetrahedralization)
  if (_dimension == 2)
  {
    InputParameters params = _factory.getValidParams("XYDelaunayGenerator");
    params.set<MeshGeneratorName>("boundary") = enclosure_surface_mg;
    params.set<BoundaryName>("output_boundary") = name() + "_outer";
    params.set<std::vector<MeshGeneratorName>>("holes") = holes_mgs;
    params.set<std::vector<BoundaryName>>("hole_boundaries") = holes_boundaries;
    params.set<Real>("desired_area") = _target_elem_volume;
    if (isParamValid("block"))
    {
      const auto block_name = getParam<SubdomainName>("block");
      params.set<SubdomainName>("output_subdomain_name") = block_name;
      _blocks.push_back(block_name);
    }
    params.set<bool>("output") = _verbose;
    params.set<bool>("show_info") = _verbose;
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "XYDelaunayGenerator", name() + "_triangulation", params);
    _mg_names.push_back(name() + "_triangulation");
  }

  // Now stitch to these components' meshes. The components may already be part of one or
  // more larger group(s) of components so we retrieve the mesh generators creating those
  std::set<MeshGeneratorName> mgs_to_stitch;
  for (const auto & comp_name : _components_to_enclose)
    mgs_to_stitch.insert(
        _awh.getAction<ActionComponent>(comp_name).getCurrentTopLevelMeshGeneratorName());

  // Merge all the outer boundaries into a single external boundary.
  std::vector<MeshGeneratorName> mgs_to_stitch_vec;
  mgs_to_stitch_vec.push_back(_mg_names.back());
  std::vector<std::vector<std::string>> final_stitching_boundaries;
  for (const auto & final_mg : mgs_to_stitch)
  {
    const auto fused_boundary = final_mg + "_fused_outer_bdy";

    // Gather outer boundaries for all
    // components sharing the same top mesh generator
    std::vector<BoundaryName> gathered_exerior_bdies;
    for (const auto & comp_name : _components_to_enclose)
      if (_awh.getAction<ActionComponent>(comp_name).getCurrentTopLevelMeshGeneratorName() ==
          final_mg)
        for (const auto & bdy_name :
             _awh.getAction<ActionComponent>(comp_name).outerSurfaceBoundaries())
          gathered_exerior_bdies.push_back(bdy_name);

    // RenameBoundaryGenerator does not keep the old boundary
    InputParameters params = _factory.getValidParams("ParsedGenerateSideset");
    params.set<MeshGeneratorName>("input") = final_mg;
    params.set<std::string>("combinatorial_geometry") = "x > -1e100";
    params.set<std::vector<BoundaryName>>("included_boundaries") = gathered_exerior_bdies;
    params.set<BoundaryName>("new_sideset_name") = fused_boundary;
    params.set<bool>("output") = _verbose;
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "ParsedGenerateSideset", final_mg + "_fused_exterior_surfaces", params);
    _mg_names.push_back(final_mg + "_fused_exterior_surfaces");

    // Form stitching pairs for each final mesh generator
    final_stitching_boundaries.push_back({"hole_bdy_" + final_mg, fused_boundary});
    // These MGs with the fused surface serve as inputs to the stitcher
    mgs_to_stitch_vec.push_back(final_mg + "_fused_exterior_surfaces");
  }

  InputParameters stitcher_params = _factory.getValidParams("StitchMeshGenerator");
  stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") = mgs_to_stitch_vec;
  stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") =
      final_stitching_boundaries;
  stitcher_params.set<bool>("enforce_all_nodes_match_on_boundaries") = false;
  stitcher_params.set<bool>("clear_stitched_boundary_ids") = false;
  stitcher_params.set<bool>("verbose_stitching") = _verbose;
  stitcher_params.set<bool>("verbose_remapping") = _verbose;
  stitcher_params.set<bool>("output") = _verbose;
  stitcher_params.set<bool>("show_info") = _verbose;
  _app.getMeshGeneratorSystem().addMeshGenerator(
      "StitchMeshGenerator", name() + "_stitched", stitcher_params);
  _mg_names.push_back(name() + "_stitched");

  // The triangulation that goes around the component could be overlapping with other components
  // Delete the elements overlapping.
  // We use the connected components, and we delete one by one as the DeleteElementsNearMeshGenerator
  // forms a giant KD-Tree with side Qps to create the deletion criterion
  // TODO: add a check on other component meshes (can already be done with diagnostics) or
  // add an option to delete more (can already be done with an additional MG in Mesh block)
  for (const auto & comp_name : _components_to_enclose)
  {
    const auto & component = _awh.getAction<ActionComponent>(comp_name);
    const auto & connected_components = component.getConnectedComponents();
    for (const auto & conn_comp : connected_components)
    {
      const auto & conn_name = conn_comp->name();
      // don't delete in proximity to the enclosed components
      if (std::find(_components_to_enclose.begin(), _components_to_enclose.end(), conn_name) !=
          _components_to_enclose.end())
        continue;
      // TODO: avoid deleting near the same connected component twice

      InputParameters proxy_del_params = _factory.getValidParams("DeleteElementsNearMeshGenerator");
      proxy_del_params.set<MeshGeneratorName>("input") = _mg_names.back();
      // only delete from the cavity
      proxy_del_params.set<std::vector<SubdomainName>>("blocks_included") = _blocks;
      // reform the external boundary
      proxy_del_params.set<BoundaryName>("new_boundary") = name() + "_outer";
      // TODO fixme
      if (std::dynamic_cast<ComponentJunction *>(conn_comp))
        proxy_del_params.set<MeshGeneratorName>("proximity_mesh") = conn_comp->meshGeneratorNames()[conn_comp->meshGeneratorNames().size() - 2];
      else
        proxy_del_params.set<MeshGeneratorName>("proximity_mesh") = conn_comp->meshGeneratorNames().back();
      proxy_del_params.set<Real>("distance") = 1e-10;
      // MooseEnum side_order("CONSTANT");
      // proxy_del_params.set<MooseEnum>("side_order") = side_order;
      proxy_del_params.set<bool>("output") = _verbose;
      proxy_del_params.set<bool>("show_info") = _verbose;
      const auto mg_name = name() + "_rm_overlap_" + conn_name;
      _app.getMeshGeneratorSystem().addMeshGenerator(
          "DeleteElementsNearMeshGenerator", mg_name, proxy_del_params);
      _mg_names.push_back(mg_name);
    }
  }

  _top_mg_name = _mg_names.back();
}

void
CavityComponent::checkIntegrity()
{
  ComponentInitialConditionInterface::checkIntegrity();
  ComponentBoundaryConditionInterface::checkIntegrity();
}
