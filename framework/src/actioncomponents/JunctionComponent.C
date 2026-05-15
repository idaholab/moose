//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "JunctionComponent.h"
#include "MooseUtils.h"

registerMooseAction("MooseApp", JunctionComponent, "add_mesh_generator");
// JunctionComponent is an example of ComponentPhysicsInterface
registerMooseAction("MooseApp", JunctionComponent, "init_component_physics");
// JunctionComponent is an example of ComponentMaterialPropertyInterface
registerMooseAction("MooseApp", JunctionComponent, "add_material");
// JunctionComponent is an example of ComponentInitialConditionInterface
registerMooseAction("MooseApp", JunctionComponent, "check_integrity");
registerActionComponent("MooseApp", JunctionComponent);

InputParameters
JunctionComponent::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += ComponentPhysicsInterface::validParams();
  params += ComponentMaterialPropertyInterface::validParams();
  params += ComponentInitialConditionInterface::validParams();
  params += ComponentBoundaryConditionInterface::validParams();
  params += ComponentMeshTransformHelper::validParams();

  params.addClassDescription("Component to join two other components.");

  params.addRequiredParam<ComponentName>("first_component", "First component to join");
  params.addRequiredParam<BoundaryName>("first_boundary", "First boundary to connect to.");
  params.addRequiredParam<ComponentName>("second_component", "Second component to join");
  params.addRequiredParam<BoundaryName>("second_boundary", "Second boundary to connect to.");

  MooseEnum junction_type("stitch_meshes mesh_gap", "mesh_gap");
  params.addParam<MooseEnum>("junction_method", junction_type, "How to join the two components");

  /* Stitching parameters */
  params.addParam<bool>("enforce_all_nodes_match_on_boundaries",
                        true,
                        "Only stitch if all nodes match on the boundary. Defaults to true because "
                        "there is a search algorithm that forces nodes to match assuming there are "
                        "equal number of nodes on each target boundary.");

  /* Meshing the gap parameters */
  // Parameters for the region between meshes
  params.addParam<unsigned int>("n_elem_normal",
                                "Number of elements in the normal direction of the junction");
  params.addParam<SubdomainName>("block", "Block name for the junction, if a block is created.");

  // Parameters for changing radius -- final radius will be calculated under the hood
  MooseEnum radial_growth_methods("LINEAR CUBIC", "CUBIC");
  params.addParam<MooseEnum>("radial_growth_method",
                             radial_growth_methods,
                             "Functional form to change radius while extruding along curve.");
  params.addParam<Real>("start_radial_growth_rate", 0, "Starting rate of radial expansion.");
  params.addParam<Real>("end_radial_growth_rate", 0, "Ending rate of radial expansion.");

  // Parameters for the 1D spline joining the two components (serving as an extrusion guide for
  // 2,3D)
  params.addRangeCheckedParam<libMesh::Real>(
      "sharpness", "sharpness>0 & sharpness<=1", "Sharpness of curve bend.");
  params.addParam<unsigned int>(
      "num_cps",
      6,
      "Number of control points used to draw the curve. Miniumum of degree+1 points are required.");
  MooseEnum edge_elem_type("EDGE2 EDGE3 EDGE4", "EDGE2");
  params.addParam<MooseEnum>(
      "edge_element_type", edge_elem_type, "Type of the EDGE elements to be generated.");

  params.addParamNamesToGroup("sharpness num_cps edge_element_type", "1D mesh junction");
  params.addParamNamesToGroup(
      "radial_growth_method start_radial_growth_rate end_radial_growth_rate",
      "Radial expansion in 2D and 3D junction");

  return params;
}

JunctionComponent::JunctionComponent(const InputParameters & params)
  : ActionComponent(params),
    ComponentPhysicsInterface(params),
    ComponentMaterialPropertyInterface(params),
    ComponentInitialConditionInterface(params),
    ComponentBoundaryConditionInterface(params),
    ComponentMeshTransformHelper(params),
    _junction_method(getParam<MooseEnum>("junction_method")),
    _enforce_all_nodes_match_on_boundaries(getParam<bool>("enforce_all_nodes_match_on_boundaries"))
{
  addRequiredTask("add_mesh_generator");

  // Check parameters
  if (_junction_method != "mesh_gap")
    errorDependentParameter("junction_method",
                            "mesh_gap",
                            {"n_elem_normal",
                             "block",
                             "radial_growth_method",
                             "start_radial_growth_rate",
                             "end_radial_growth_rate",
                             "sharpness",
                             "num_cps",
                             "edge_element_type"});
  // The 1D and 2D meshing parameters will be re-checked later, once we know the dimension
}

void
JunctionComponent::addMeshGenerators()
{
  const auto & first_component =
      _awh.getAction<ActionComponent>(getParam<ComponentName>("first_component"));
  const auto & second_component =
      _awh.getAction<ActionComponent>(getParam<ComponentName>("second_component"));
  const auto first_boundary = getParam<BoundaryName>("first_boundary");
  const auto second_boundary = getParam<BoundaryName>("second_boundary");

  // Get the dimension of the components
  const auto dimension_first = first_component.dimension();
  const auto dimension_second = second_component.dimension();

  if (dimension_first == 0 || dimension_second == 0)
    mooseError("Connecting 0 dimension meshes not implemented!");

  // Perform junction
  if (_junction_method == "stitch_meshes")
  {
    // Fairly easy to stitch this
    if (dimension_first == dimension_second)
    {
      if (getParam<ComponentName>("first_component") != getParam<ComponentName>("second_component"))
      { // Stitch the two meshes
        InputParameters params = _factory.getValidParams("StitchMeshGenerator");
        params.set<std::vector<MeshGeneratorName>>("inputs") = {first_component.mg_names().back(),
                                                                second_component.mg_names().back()};
        params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
            {first_boundary, second_boundary}};
        params.set<bool>("verbose_stitching") = _verbose;
        params.set<bool>("enforce_all_nodes_match_on_boundaries") =
            _enforce_all_nodes_match_on_boundaries;
        _app.getMeshGeneratorSystem().addMeshGenerator(
            "StitchMeshGenerator", name() + "_base", params);
        _mg_names.push_back(name() + "_base");
      }
      else
      {
        // Handles case of needing to close a mesh. Using StitchBoundaryMeshGenerator prevents
        // issues with element overlap.

        InputParameters params = _factory.getValidParams("StitchBoundaryMeshGenerator");
        params.set<MeshGeneratorName>("input") = first_component.mg_names().back();
        params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
            {first_boundary, second_boundary}};
        params.set<bool>("show_info") = _verbose;
        _app.getMeshGeneratorSystem().addMeshGenerator(
            "StitchBoundaryMeshGenerator", name() + "_close", params);
        _mg_names.push_back(name() + "_close");
      }
    }
    else
      mooseError("Stiching meshes of different dimensions is not implemented");
  }

  else if (_junction_method == "mesh_gap")
  {
    //
    // This method is set to use a B-Spline to draw a 1D curve between
    //

    // not all action components are guaranteed to inherit from ComponentMeshTransformerHelper,
    // which we need to specify the direction.
    // TODO: modify the spline curve generator to figure out the direction so we can support other
    // component types
    const auto & first_component_cmth =
        _awh.getAction<ComponentMeshTransformHelper>(getParam<ComponentName>("first_component"));
    const auto & second_component_cmth =
        _awh.getAction<ComponentMeshTransformHelper>(getParam<ComponentName>("second_component"));

    // find start and end directions (may need to take the negative of the end direction).
    // obey user parameters if set

    RealVectorValue start_direction = first_component_cmth.direction();
    RealVectorValue end_direction = second_component_cmth.direction();

    InputParameters bspline_params = _factory.getValidParams("BSplineCurveGenerator");
    bspline_params.set<RealVectorValue>("start_direction") = start_direction;
    bspline_params.set<RealVectorValue>("end_direction") = -end_direction;
    bspline_params.set<unsigned int>("num_elements") = getParam<unsigned int>("n_elem_normal");
    if (isParamValid("sharpness"))
      bspline_params.set<Real>("sharpness") = getParam<Real>("sharpness");
    bspline_params.set<unsigned int>("num_cps") = getParam<unsigned int>("num_cps");
    bspline_params.set<MeshGeneratorName>("start_mesh") = first_component.mg_names().back();
    bspline_params.set<MeshGeneratorName>("end_mesh") = second_component.mg_names().back();
    bspline_params.set<BoundaryName>("boundary_providing_start_point") =
        getParam<BoundaryName>("first_boundary");
    bspline_params.set<BoundaryName>("boundary_providing_end_point") =
        getParam<BoundaryName>("second_boundary");

    if (dimension_first == 1)
    {
      if (isParamValid("block"))
        bspline_params.set<BoundaryName>("new_subdomain_name") = getParam<SubdomainName>("block");
      bspline_params.set<std::vector<BoundaryName>>("edge_nodesets") = {
          name() + "_bspline_start_node", name() + "_bspline_end_node"};
    }

    _app.getMeshGeneratorSystem().addMeshGenerator(
        "BSplineCurveGenerator", name() + "_curve", bspline_params);
    _mg_names.push_back(name() + "_curve");

    // Extrude boundary from first component
    if (dimension_first > 1)
    {
      // create lower dimension mesh on the boundary
      InputParameters ld_source_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
      ld_source_params.set<MeshGeneratorName>("input") = first_component.mg_names().back();
      ld_source_params.set<std::vector<BoundaryName>>("sidesets") =
          std::vector{getParam<BoundaryName>("first_boundary")};
      ld_source_params.set<SubdomainName>("new_block_name") =
          (SubdomainName)(name() + "_LowerDBlockSource");
      _app.getMeshGeneratorSystem().addMeshGenerator(
          "LowerDBlockFromSidesetGenerator", name() + "_lowerDGenerationSource", ld_source_params);
      _mg_names.push_back(name() + "_lowerDGenerationSource");

      InputParameters _bmc_source_params = _factory.getValidParams("BlockToMeshConverterGenerator");
      _bmc_source_params.set<MeshGeneratorName>("input") = name() + "_lowerDGenerationSource";
      _bmc_source_params.set<std::vector<SubdomainName>>("target_blocks") = {
          (SubdomainName)(name() + "_LowerDBlockSource")};
      _app.getMeshGeneratorSystem().addMeshGenerator(
          "BlockToMeshConverterGenerator", name() + "_blockToMeshSource", _bmc_source_params);
      _mg_names.push_back(name() + "_blockToMeshSource");

      // set up AdvancedExtruderGenerator
      InputParameters aeg_params = _factory.getValidParams("AdvancedExtruderGenerator");
      aeg_params.set<MeshGeneratorName>("extrusion_curve") = (MeshGeneratorName)(name() + "_curve");
      aeg_params.set<MeshGeneratorName>("input") =
          (MeshGeneratorName)(name() + "_blockToMeshSource");

      aeg_params.set<RealVectorValue>("start_extrusion_direction") = start_direction;
      aeg_params.set<RealVectorValue>("end_extrusion_direction") = end_direction;

      aeg_params.set<Real>("start_radial_growth_rate") = getParam<Real>("start_radial_growth_rate");
      aeg_params.set<Real>("end_radial_growth_rate") = getParam<Real>("end_radial_growth_rate");
      aeg_params.set<MooseEnum>("radial_growth_method") =
          getParam<MooseEnum>("radial_growth_method");

      aeg_params.set<BoundaryName>("bottom_boundary") = name() + "_aeg_bottom_boundary";
      aeg_params.set<BoundaryName>("top_boundary") = name() + "_aeg_top_boundary";
      if (isParamValid("block"))
        paramError("block", "Not yet implemented for 2D or 3D junction");
      aeg_params.set<bool>("output") = _verbose;

      _app.getMeshGeneratorSystem().addMeshGenerator(
          "AdvancedExtruderGenerator", name() + "_aeg", aeg_params);
      _mg_names.push_back(name() + "_aeg");
    }

    // Stitch the extrusion / curve (in 1D) to the components
    if (getParam<ComponentName>("first_component") != getParam<ComponentName>("second_component"))
    {
      InputParameters stitcher_params = _factory.getValidParams("StitchMeshGenerator");
      stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") =
          std::vector<MeshGeneratorName>{first_component.mg_names().back(),
                                         name() + "_aeg",
                                         second_component.mg_names().back()};
      if (dimension_first > 1)
        stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
            {first_boundary, name() + "_aeg_bottom_boundary"},
            {name() + "_aeg_top_boundary", second_boundary}};
      else
        stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
            {first_boundary, name() + "_bspline_start_node"},
            {name() + "_bspline_end_node", second_boundary}};

      stitcher_params.set<bool>("verbose_stitching") = _verbose;
      stitcher_params.set<bool>("output") = _verbose;
      stitcher_params.set<bool>("enforce_all_nodes_match_on_boundaries") =
          _enforce_all_nodes_match_on_boundaries;
      _app.getMeshGeneratorSystem().addMeshGenerator(
          "StitchMeshGenerator", name() + "_stitcher", stitcher_params);
      _mg_names.push_back(name() + "_stitcher");
    }
    else
    {
      // Handles case of needing to close a mesh. Using StitchBoundaryMeshGenerator prevents
      // issues with element overlap.
      InputParameters mesh_stitcher_params = _factory.getValidParams("StitchMeshGenerator");
      mesh_stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") =
          std::vector<MeshGeneratorName>{first_component.mg_names().back(), name() + "_aeg"};
      if (dimension_first > 1)
        mesh_stitcher_params.set<std::vector<std::vector<std::string>>>(
            "stitch_boundaries_pairs") = {{first_boundary, name() + "_aeg_bottom_boundary"}};
      else
        mesh_stitcher_params.set<std::vector<std::vector<std::string>>>(
            "stitch_boundaries_pairs") = {{first_boundary, name() + "_bspline_start_node"}};
      mesh_stitcher_params.set<bool>("verbose_stitching") = _verbose;
      mesh_stitcher_params.set<bool>("enforce_all_nodes_match_on_boundaries") =
          _enforce_all_nodes_match_on_boundaries;
      _app.getMeshGeneratorSystem().addMeshGenerator(
          "StitchMeshGenerator", name() + "_mesh_stitcher", mesh_stitcher_params);
      _mg_names.push_back(name() + "_mesh_stitcher");

      InputParameters boundary_stitcher_params =
          _factory.getValidParams("StitchBoundaryMeshGenerator");
      boundary_stitcher_params.set<MeshGeneratorName>("input") = name() + "_mesh_stitcher";
      boundary_stitcher_params.set<std::vector<std::vector<std::string>>>(
          "stitch_boundaries_pairs") = {{name() + "_aeg_top_boundary", second_boundary}};
      boundary_stitcher_params.set<bool>("show_info") = _verbose;
      _app.getMeshGeneratorSystem().addMeshGenerator(
          "StitchBoundaryMeshGenerator", name() + "_closed", boundary_stitcher_params);
      _mg_names.push_back(name() + "_closed");
    }
  }
  else
    mooseError("junction_method specified is invalid!");
}

void
JunctionComponent::checkIntegrity()
{
  ComponentInitialConditionInterface::checkIntegrity();
  ComponentBoundaryConditionInterface::checkIntegrity();
}
