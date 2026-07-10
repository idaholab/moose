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
  params += ComponentMeshGenerationHelper::validParams();
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
  params.addParam<unsigned int>(
      "n_boundary_layers",
      0,
      "Number of boundary layers in between the components and the bulk mesh of the cavity");
  params.addParam<Real>("boundary_layer_width", "Total width of the meshed boundary layer");

  params.addParamNamesToGroup("target_element_volume n_boundary_layers boundary_layer_width",
                              "Discretization");
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
    ComponentMeshGenerationHelper(params),
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
  std::vector<BoundaryName> cavity_inner_boundaries = holes_boundaries;

  // Displace the surface mesh if requested
  MeshGeneratorName enclosure_surface_mg = _surface_mesh_mg;
  if (isParamSetByUser("recenter_surface_mesh_to"))
  {
    // Re-center the mesh to the origin
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _surface_mesh_mg;
    params.set<MooseEnum>("transform") = "TRANSLATE_CENTER_ORIGIN";
    addMeshGenerator("TransformGenerator", "translated_to_origin", params);

    // Re-center to the user-input coordinates
    params.set<MeshGeneratorName>("input") = _mg_names.back();
    params.set<MooseEnum>("transform") = "TRANSLATE";
    params.set<RealVectorValue>("vector_value") = _center_surface_mesh;
    addMeshGenerator("TransformGenerator", "translated", params);
    enclosure_surface_mg = _mg_names.back();
  }

  // Build boundary layers before filling the surface / volume
  if (getParam<unsigned int>("n_boundary_layers") > 0)
  {
    // Gather outer boundaries
    std::vector<BoundaryName> gathered_exerior_bdies;
    MeshGeneratorName comp_final_mg = "";
    for (const auto & comp_name : _components_to_enclose)
    {
      const auto & component = _awh.getAction<ActionComponent>(comp_name);
      if (comp_final_mg == "")
        comp_final_mg = component.getCurrentTopLevelMeshGeneratorName();
      else if (comp_final_mg != component.getCurrentTopLevelMeshGeneratorName())
        paramError("enclosed_components",
                   "Disconnected components not supported for boundary layers at this time");

      for (const auto & bdy_name : component.outerSurfaceBoundaries())
        gathered_exerior_bdies.push_back(bdy_name);
    }

    // Extract the surface into a mesh, the format that the extruder expects it
    InputParameters lowD_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
    lowD_params.set<MeshGeneratorName>("input") = comp_final_mg;
    const auto lowD_block_name = name() + "lowD_for_boundary_layer";
    lowD_params.set<SubdomainName>("new_block_name") = lowD_block_name;
    lowD_params.set<std::vector<BoundaryName>>("sidesets") = gathered_exerior_bdies;
    addMeshGenerator("LowerDBlockFromSidesetGenerator", "create_lowerD", lowD_params);

    InputParameters blockToMesh_params = _factory.getValidParams("BlockToMeshConverterGenerator");
    blockToMesh_params.set<MeshGeneratorName>("input") = _mg_names.back();
    blockToMesh_params.set<std::vector<SubdomainName>>("target_blocks") = {lowD_block_name};
    addMeshGenerator("BlockToMeshConverterGenerator", "lowerD_into_block", blockToMesh_params);

    // Create a boundary layer by extruding from the component surfaces
    // NOTE: this should become an option, a parameter we can turn on
    InputParameters ext_params = _factory.getValidParams("AdvancedExtruderGenerator");
    ext_params.set<std::vector<unsigned int>>("num_layers") = {
        getParam<unsigned int>("n_boundary_layers")};
    ext_params.set<MeshGeneratorName>("input") = _mg_names.back();
    if (isParamValid("boundary_layer_width"))
      ext_params.set<std::vector<Real>>("heights") = {getParam<Real>("boundary_layer_width")};
    else
      paramError("boundary_layer_width", "Must be passed if n_boundary_layers > 0");
    ext_params.set<BoundaryName>("bottom_boundary") = name() + "_base_layer";
    ext_params.set<BoundaryName>("top_boundary") = name() + "_boundary_layers";
    ext_params.set<bool>("extrude_along_node_normals") = true;
    addMeshGenerator("AdvancedExtruderGenerator", "boundary_layers", ext_params);
    const auto blayer_mg_name = _mg_names.back();

    if (_dimension == 3)
    {
      // Add a transition layer on the side that will connect to the tets
      InputParameters tet_layer_params =
          _factory.getValidParams("BoundaryElementConversionGenerator");
      tet_layer_params.set<MeshGeneratorName>("input") = _mg_names.back();
      tet_layer_params.set<std::vector<BoundaryName>>("boundary_names") = {name() +
                                                                           "_boundary_layers"};
      addMeshGenerator(
          "BoundaryElementConversionGenerator", "tet_transition_layer", tet_layer_params);
    }

    // replace the holes and boundaries
    // NOTE: we only support a single connected (by mesh generation) component at this time
    holes_mgs.clear();
    // we are going to rely on BoundaryElementConversionGenerator and all_tri giving the same
    // tetrahedralization of the boundary here... because all_tri can't handle pyramids at this time
    // TODO: see https://github.com/libMesh/libmesh/issues/4495
    holes_mgs.push_back(blayer_mg_name);
    holes_boundaries.clear();
    holes_boundaries.push_back(name() + "_boundary_layers");
    cavity_inner_boundaries.clear();
    cavity_inner_boundaries.push_back(name() + "_base_layer");

    if (isParamValid("block"))
    {
      const auto block_name = getParam<SubdomainName>("block");
      _blocks.push_back(block_name + "_to_tet");
      _blocks.push_back(block_name + "_to_pyramid");
    }
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
    addMeshGenerator("XYDelaunayGenerator", "triangulation", params);
  }
  else
  {
    // Mesh the cavity with tets
    InputParameters params = _factory.getValidParams("XYZDelaunayGenerator");
    params.set<MeshGeneratorName>("boundary") = enclosure_surface_mg;
    params.set<BoundaryName>("output_boundary") = name() + "_outer";
    params.set<std::vector<MeshGeneratorName>>("holes") = holes_mgs;
    params.set<std::vector<BoundaryName>>("hole_boundaries") = holes_boundaries;
    params.set<Real>("desired_volume") = _target_elem_volume;
    if (isParamValid("block"))
    {
      const auto block_name = getParam<SubdomainName>("block");
      params.set<SubdomainName>("output_subdomain_name") = block_name;
      _blocks.push_back(block_name);
    }
    // we don't stitch to the boundary layer here because the conversion methods don't handle
    // triangulating on only one exterior side
    params.set<MooseEnum>("conversion_method") = "SURFACE";
    params.set<bool>("convert_holes_for_stitching") = true;
    // we can stitch to the components directly when not using a boundary layer
    if (getParam<unsigned int>("n_boundary_layers") == 0)
      params.set<std::vector<bool>>("stitch_holes") = std::vector<bool>(holes_mg.size(), true);
    addMeshGenerator("XYZDelaunayGenerator", "tetrahedralization", params);

    // Stitch the tetrahedralization to the boundary layer
    // The boundary layer should have a transition layer too
    if (getParam<unsigned int>("n_boundary_layers") > 0)
    {
      InputParameters stitcher_params = _factory.getValidParams("StitchMeshGenerator");
      stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") = {*(_mg_names.rbegin() + 1),
                                                                       _mg_names.back()};
      stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
          {holes_boundaries.back(), holes_boundaries.back()}};
      stitcher_params.set<bool>("enforce_all_nodes_match_on_boundaries") = true;
      stitcher_params.set<bool>("clear_stitched_boundary_ids") = false;
      stitcher_params.set<bool>("verbose_stitching") = _verbose;
      stitcher_params.set<bool>("verbose_remapping") = _verbose;
      addMeshGenerator("StitchMeshGenerator", "stitched_bl", stitcher_params);
    }
  }

  if (getParam<unsigned int>("n_boundary_layers") > 0)
  {
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
    for (const auto & i_mg : index_range(mgs_to_stitch))
    {
      const auto & final_mg = *std::next(mgs_to_stitch.begin(), i_mg);
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
      addMeshGenerator("ParsedGenerateSideset", final_mg + "_fused_exterior_surfaces", params);

      // Form stitching pairs for each final mesh generator
      final_stitching_boundaries.push_back({fused_boundary, cavity_inner_boundaries[i_mg]});
      // These MGs with the fused surface serve as inputs to the stitcher
      mgs_to_stitch_vec.insert(mgs_to_stitch_vec.begin(), _mg_names.back());
    }

    // Stitch the (cavity + boundary) layers to the components
    InputParameters stitcher_params = _factory.getValidParams("StitchMeshGenerator");
    stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") = mgs_to_stitch_vec;
    stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") =
        final_stitching_boundaries;
    stitcher_params.set<bool>("enforce_all_nodes_match_on_boundaries") = false;
    stitcher_params.set<bool>("clear_stitched_boundary_ids") = false;
    stitcher_params.set<bool>("verbose_stitching") = _verbose;
    stitcher_params.set<bool>("verbose_remapping") = _verbose;
    addMeshGenerator("StitchMeshGenerator", "stitched", stitcher_params);
  }

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

      InputParameters proxy_del_params = _factory.getValidParams("DeleteElementsNearMeshGenerator");
      proxy_del_params.set<MeshGeneratorName>("input") = _mg_names.back();
      // only delete from the cavity
      proxy_del_params.set<std::vector<SubdomainName>>("blocks_included") = _blocks;
      // reform the external boundary
      proxy_del_params.set<BoundaryName>("new_boundary") = name() + "_outer";
      proxy_del_params.set<MeshGeneratorName>("proximity_mesh") =
          conn_comp->getOwnMeshMeshGeneratorName();
      proxy_del_params.set<Real>("distance") = 1e-10;
      const auto mg_name = name() + "_rm_overlap_" + conn_name;
      // Avoid deleting if connected component does not have a mesh
      // Avoid adding the same deletion next to the same connected component twice
      if (!conn_comp->getOwnMeshMeshGeneratorName().empty() &&
          std::find(_mg_names.begin(), _mg_names.end(), mg_name) == _mg_names.end())
      {
        addMeshGenerator(
            "DeleteElementsNearMeshGenerator", "rm_overlap_" + conn_name, proxy_del_params);
      }
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
