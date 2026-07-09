//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SurfaceExtrusionComponent.h"
#include "RotationMatrix.h"

registerMooseAction("MooseApp", SurfaceExtrusionComponent, "add_mesh_generator");
// SurfaceExtrusionComponent is an example of ComponentPhysicsInterface
registerMooseAction("MooseApp", SurfaceExtrusionComponent, "init_component_physics");
// SurfaceExtrusionComponent is an example of ComponentMaterialPropertyInterface
registerMooseAction("MooseApp", SurfaceExtrusionComponent, "add_material");
// SurfaceExtrusionComponent is an example of ComponentInitialConditionInterface
registerMooseAction("MooseApp", SurfaceExtrusionComponent, "check_integrity");
registerActionComponent("MooseApp", SurfaceExtrusionComponent);

InputParameters
SurfaceExtrusionComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += ComponentPhysicsInterface::validParams();
  params += ComponentMaterialPropertyInterface::validParams();
  params += ComponentInitialConditionInterface::validParams();
  params += ComponentBoundaryConditionInterface::validParams();
  params += ComponentMeshGenerationHelper::validParams();
  params.addClassDescription(
      "Component for the volume created by extruding from another component's boundary.");
  params.addParam<SubdomainName>("block", "Subdomain name for the component mesh");

  // Geometry
  params.addRequiredParam<ComponentName>(
      "connected_component", "Name of the component providing the surface to extrude from");
  params.addRequiredParam<BoundaryName>(
      "source_surface", "Name of the surface on the connected component's mesh to extrude from");
  params.addRequiredRangeCheckedParam<Real>("length", "length>0", "Length of the extrusion");
  params.addRequiredParam<RealVectorValue>("direction", "Direction to extrude the surface into");

  // Radial expansion
  params.addParam<Real>(
      "start_radial_extent",
      "If specified, sets a starting reference radial extent for radial expansion of the surface "
      "during extrusion. The 'direction' points away from the 'start' of the extrusion.");
  params.addParam<Real>(
      "end_radial_extent",
      "If specified, sets a ending radial extent for radial expansion of the surface during "
      "extrusion. The 'direction' points to the 'end' of the extrusion.");
  MooseEnum radial_growth_methods("linear cubic", "linear");
  params.addParam<MooseEnum>("radial_growth_method",
                             radial_growth_methods,
                             "Functional form to change radial extent while extruding. The "
                             "extrusion axis is formed by the direction and the surface centroid");
  params.addParam<Real>(
      "start_radial_growth_rate", 1., "Starting rate of radial expansion of the extruded mesh.");
  params.addParam<Real>(
      "end_radial_growth_rate", 1., "Ending rate of radial expansion of the extruded mesh.");

  // Discretization
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_axial", "n_axial>0", "Number of axial elements of the extrusion");

  params.addParamNamesToGroup("n_axial", "Discretization");
  params.addParamNamesToGroup("start_radial_extent end_radial_extent radial_growth_method "
                              "start_radial_growth_rate end_radial_growth_rate",
                              "Radial expansion");

  return params;
}

SurfaceExtrusionComponent::SurfaceExtrusionComponent(const InputParameters & params)
  : ActionComponent(params),
    ComponentPhysicsInterface(params),
    ComponentMaterialPropertyInterface(params),
    ComponentInitialConditionInterface(params),
    ComponentBoundaryConditionInterface(params),
    ComponentMeshGenerationHelper(params),
    _length(getParam<Real>("length"))
{
  // The other component interfaces add their required task
  addRequiredTask("add_mesh_generator");

  // Parameter checks
  checkSecondParamSetOnlyIfFirstOneSet("end_radial_extent", "start_radial_extent");
  checkSecondParamSetOnlyIfFirstOneSet("end_radial_extent", "radial_growth_method");
  if (getParam<MooseEnum>("radial_growth_method") != "CUBIC")
    errorDependentParameter(
        "radial_growth_method", "CUBIC", {"start_radial_growth_rate", "end_radial_growth_rate"});
}

void
SurfaceExtrusionComponent::addMeshGenerators()
{
  // Set outer boundaries
  // The surface the extrusion starts from is not considered external at this time
  // TODO: we are missing the outer "radial" boundary
  _outer_boundaries = {name() + "_top_boundary"};

  // Assuming we always extrude dim-1 sidesets, this should work
  _dimension =
      _awh.getAction<ActionComponent>(getParam<ComponentName>("connected_component")).dimension();

  auto & connected_comp =
      _awh.getAction<ActionComponent>(getParam<ComponentName>("connected_component"));

  // Create the base mesh for the component using a mesh generator
  if (_dimension <= 1)
    paramError("dimension", "0D and 1D surface extrusion not implemented");
  else
  {
    // Extract the surface mesh
    InputParameters lowD_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
    // use the own mesh generator to get the smallest mesh suitable
    lowD_params.set<MeshGeneratorName>("input") = connected_comp.getOwnMeshMeshGeneratorName();
    const auto lowD_block_name = name() + "lowD_for_extrusion";
    lowD_params.set<SubdomainName>("new_block_name") = lowD_block_name;
    lowD_params.set<std::vector<BoundaryName>>("sidesets") = {
        getParam<BoundaryName>("source_surface")};
    addMeshGenerator("LowerDBlockFromSidesetGenerator", "create_lowerD", lowD_params);

    InputParameters blockToMesh_params = _factory.getValidParams("BlockToMeshConverterGenerator");
    blockToMesh_params.set<MeshGeneratorName>("input") = _mg_names.back();
    blockToMesh_params.set<std::vector<SubdomainName>>("target_blocks") = {lowD_block_name};
    addMeshGenerator("BlockToMeshConverterGenerator", "lowerD_into_block", blockToMesh_params);

    // Extrude the surface to form this component's mesh
    InputParameters ext_params = _factory.getValidParams("AdvancedExtruderGenerator");
    ext_params.set<std::vector<unsigned int>>("num_layers") = {getParam<unsigned int>("n_axial")};
    ext_params.set<MeshGeneratorName>("input") = _mg_names.back();
    ext_params.set<std::vector<Real>>("heights") = {_length};
    // used for stitching
    ext_params.set<BoundaryName>("bottom_boundary") = name() + "_bottom_boundary";
    ext_params.set<BoundaryName>("top_boundary") = name() + "_top_boundary";
    if (isParamValid("end_radial_extent"))
    {
      // avoid computing the radial extent from the mesh and suffering from polygonization
      if (isParamValid("start_radial_extent"))
        ext_params.set<Real>("start_radial_extent") = getParam<Real>("start_radial_extent");
      ext_params.set<Real>("end_radial_extent") = getParam<Real>("end_radial_extent");
      const auto & growth_method = getParam<MooseEnum>("radial_growth_method");
      ext_params.set<MooseEnum>("radial_growth_method") = growth_method;

      // Set growth rates for cubic
      if (growth_method == "CUBIC")
        ext_params.applySpecificParameters(parameters(),
                                           {"start_radial_growth_rate", "end_radial_growth_rate"});
    }
    ext_params.set<Point>("direction") = getParam<RealVectorValue>("direction");
    addMeshGenerator("AdvancedExtruderGenerator", "extrusion", ext_params);
    setOwnMeshMeshGeneratorName(_mg_names.back());

    // Stitch to the original component at the boundary we extruded from
    // NOTE: this could be optional
    InputParameters stitcher_params = _factory.getValidParams("StitchMeshGenerator");
    stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") = {
        connected_comp.getCurrentTopLevelMeshGeneratorName(), _mg_names.back()};
    stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
        {getParam<BoundaryName>("source_surface"), name() + "_bottom_boundary"}};
    // additional components might have added nodes to 'source_surface', these components'
    // meshes are included now due to using the top-level mesh generator
    stitcher_params.set<bool>("enforce_all_nodes_match_on_boundaries") = false;
    stitcher_params.set<bool>("clear_stitched_boundary_ids") = false;
    stitcher_params.set<bool>("verbose_stitching") = _verbose;
    stitcher_params.set<bool>("verbose_remapping") = _verbose;
    addMeshGenerator("StitchMeshGenerator", "stitched", stitcher_params);
  }
  _top_mg_name = _mg_names.back();

  // Connect the extrusion to the component with the extrusion source
  addConnectedComponent(connected_comp);
  connected_comp.addConnectedComponent(*this);
  // Update the top mesh generator since it's stitched into this one
  connected_comp.setCurrentTopLevelMeshGeneratorName(_top_mg_name);
  // Sets it for all connected components
  // Connected might not be the right abstraction here. It's more like "included in a common mesh"
  for (auto * component : connected_comp.getConnectedComponents())
    component->setCurrentTopLevelMeshGeneratorName(_top_mg_name);
}

void
SurfaceExtrusionComponent::checkIntegrity()
{
  ComponentInitialConditionInterface::checkIntegrity();
  ComponentBoundaryConditionInterface::checkIntegrity();
}
