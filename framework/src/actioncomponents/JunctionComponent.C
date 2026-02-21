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

  // optional user parameters. Will be obeyed always.
  params.addParam<RealVectorValue>("first_direction", "Direction from the first boundary.");
  params.addParam<RealVectorValue>("second_direction", "Direction from the second boundary.");

  MooseEnum junction_type("stitch_meshes fill_gap", "fill_gap");
  params.addParam<MooseEnum>("junction_method", junction_type, "How to join the two components");

  params.addParam<bool>("enforce_all_nodes_match_on_boundaries",
                        true,
                        "Only stitch if all nodes match on the boundary. Defaults to true because "
                        "there is a search algorithm that forces nodes to match assuming there are "
                        "equal number of nodes on each target boundary.");
  params.addParam<bool>("correct_extrusion_to_target",
                        true,
                        "When extruding along a curve to a target radius, the extruded nodes will "
                        "be corrected to align with the target mesh on the last extrusion.");

  // Parameters for changing radius -- final radius will be calculated under the hood
  MooseEnum radial_growth_methods("LINEAR CUBIC", "CUBIC");
  params.addParam<MooseEnum>("radial_growth_method",
                             radial_growth_methods,
                             "Functional form to change radius while extruding along curve.");
  params.addParam<Real>("start_radial_growth_rate", 0, "Starting rate of radial expansion.");
  params.addParam<Real>("end_radial_growth_rate", 0, "Ending rate of radial expansion.");

  // Parameters for the region between meshes
  params.addParam<unsigned int>("n_elem_normal",
                                "Number of elements in the normal direction of the junction");
  // params.addParam<SubdomainName>("block", "Block name for the junction, if a block is created.");

  params.addParam<std::vector<BoundaryName>>("1D_edge_nodesets",
                                             std::vector<BoundaryName>(),
                                             "Nodesets on both edges of the spline curve.");

  // add curve controls
  params.addRangeCheckedParam<libMesh::Real>(
      "sharpness", "sharpness>0 & sharpness<=1", "Sharpness of curve bend.");
  params.addParam<unsigned int>(
      "num_cps",
      6,
      "Number of control points used to draw the curve. Miniumum of degree+1 points are required.");

  MooseEnum edge_elem_type("EDGE2 EDGE3 EDGE4", "EDGE2");
  params.addParam<MooseEnum>(
      "edge_element_type", edge_elem_type, "Type of the EDGE elements to be generated.");

  // add parameters to an Advanced group for additional controls
  params.addParamNamesToGroup("sharpness num_cps edge_element_type radial_growth_method "
                              "start_radial_growth_rate end_radial_growth_rate",
                              "Advanced");

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
  // if (!_junction_method.contains("fill_gap"))
  //   errorDependentParameter("junction_method", "fill_gap_and_stitch", {"n_elem_normal",
  //   "block"});

  if (_junction_method == "fill_gap" && !isParamValid("n_elem_normal"))
    paramError("n_elem_normal",
               "n_elem_normal must be specified if junction_method is set to fill_gap!");
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

  if (dimension_first != dimension_second)
    mooseError("Input components do not have the same mesh dimensions!");

  _dimension = dimension_first; // set the dimension of the action component

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
        params.set<bool>("output") = _verbose;
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
    {
      std::cout << "dim_first = " << dimension_first << std::endl;
      std::cout << "dim_second = " << dimension_second << std::endl;
      mooseError("Stiching meshes of different dimensions is not implemented");
    }
  }
  else if (_junction_method == "fill_gap" && dimension_first > 1)
  {
    //
    // This method is set to use a B-Spline to draw a 1D curve between
    //

    // not all action components are guaranteed to inherit from ComponentMeshTransformerHelper,
    // which we need to specify the direction.
    const auto & first_component_cmth =
        _awh.getAction<ComponentMeshTransformHelper>(getParam<ComponentName>("first_component"));
    const auto & second_component_cmth =
        _awh.getAction<ComponentMeshTransformHelper>(getParam<ComponentName>("second_component"));

    // find start and end directions (may need to take the negative of the end direction).
    // obey user parameters if set

    RealVectorValue start_direction, end_direction;
    if (isParamValid("first_direction"))
      start_direction = getParam<RealVectorValue>("first_direction");
    else
      start_direction = first_component_cmth.direction();
    if (isParamValid("second_direction"))
      end_direction = getParam<RealVectorValue>("second_direction");
    else
      end_direction = second_component_cmth.direction();

    InputParameters bspline_params = _factory.getValidParams("BSplineCurveGenerator");
    bspline_params.set<RealVectorValue>("start_direction") = start_direction;
    bspline_params.set<RealVectorValue>("end_direction") = -end_direction;
    bspline_params.set<unsigned int>("num_elements") = getParam<unsigned int>("n_elem_normal");
    if (isParamValid("sharpness"))
      bspline_params.set<Real>("sharpness") = getParam<Real>("sharpness");
    bspline_params.set<unsigned int>("num_cps") = getParam<unsigned int>("num_cps");
    bspline_params.set<MeshGeneratorName>("start_mesh") =
        first_component.mg_names()
            .back(); // get last name from list of generators creating the component
    bspline_params.set<MeshGeneratorName>("end_mesh") =
        second_component.mg_names()
            .back(); // get last name from list of generators creating the component
    bspline_params.set<BoundaryName>("start_boundary") = getParam<BoundaryName>("first_boundary");
    bspline_params.set<BoundaryName>("end_boundary") = getParam<BoundaryName>("second_boundary");
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "BSplineCurveGenerator", name() + "_curve", bspline_params);
    _mg_names.push_back(name() + "_curve");

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

    // do the same for target boundary
    InputParameters ld_target_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
    ld_target_params.set<MeshGeneratorName>("input") = second_component.mg_names().back();
    ld_target_params.set<std::vector<BoundaryName>>("sidesets") =
        std::vector{getParam<BoundaryName>("second_boundary")};
    ld_target_params.set<SubdomainName>("new_block_name") =
        (SubdomainName)(name() + "_LowerDBlockTarget");
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "LowerDBlockFromSidesetGenerator", name() + "_lowerDGenerationTarget", ld_target_params);
    _mg_names.push_back(name() + "_lowerDGenerationTarget");

    InputParameters _bmc_target_params = _factory.getValidParams("BlockToMeshConverterGenerator");
    _bmc_target_params.set<MeshGeneratorName>("input") = name() + "_lowerDGenerationTarget";
    _bmc_target_params.set<std::vector<SubdomainName>>("target_blocks") = {
        (SubdomainName)(name() + "_LowerDBlockTarget")};
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "BlockToMeshConverterGenerator", name() + "_blockToMeshTarget", _bmc_target_params);
    _mg_names.push_back(name() + "_blockToMeshTarget");

    // set up AdvancedExtruderGenerator
    InputParameters aeg_params = _factory.getValidParams("AdvancedExtruderGenerator");
    aeg_params.set<MeshGeneratorName>("extrusion_curve") = (MeshGeneratorName)(name() + "_curve");
    aeg_params.set<MeshGeneratorName>("input") = (MeshGeneratorName)(name() + "_blockToMeshSource");
    aeg_params.set<MeshGeneratorName>("target_mesh") =
        (MeshGeneratorName)(name() + "_blockToMeshTarget");

    aeg_params.set<Point>("start_extrusion_direction") = start_direction;
    aeg_params.set<Point>("end_extrusion_direction") = end_direction;

    aeg_params.set<Real>("start_radial_growth_rate") = getParam<Real>("start_radial_growth_rate");
    aeg_params.set<Real>("end_radial_growth_rate") = getParam<Real>("end_radial_growth_rate");
    aeg_params.set<MooseEnum>("radial_growth_method") = getParam<MooseEnum>("radial_growth_method");

    aeg_params.set<BoundaryName>("bottom_boundary") = name() + "_aeg_bottom_boundary";
    aeg_params.set<BoundaryName>("top_boundary") = name() + "_aeg_top_boundary";

    aeg_params.set<bool>("correct_extrusion_to_target") =
        getParam<bool>("correct_extrusion_to_target");

    _app.getMeshGeneratorSystem().addMeshGenerator(
        "AdvancedExtruderGenerator", name() + "_aeg", aeg_params);
    _mg_names.push_back(name() + "_aeg");

    // stitcher for results
    if (getParam<ComponentName>("first_component") != getParam<ComponentName>("second_component"))
    {
      InputParameters stitcher_params = _factory.getValidParams("StitchMeshGenerator");
      stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") =
          std::vector<MeshGeneratorName>{first_component.mg_names().back(),
                                         name() + "_aeg",
                                         second_component.mg_names().back()};
      stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
          {first_boundary, name() + "_aeg_bottom_boundary"},
          {name() + "_aeg_top_boundary", second_boundary}};
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
      mesh_stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
          {first_boundary, name() + "_aeg_bottom_boundary"}};
      mesh_stitcher_params.set<bool>("verbose_stitching") = _verbose;
      mesh_stitcher_params.set<bool>("output") = _verbose;
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
  else if (_junction_method == "fill_gap" && dimension_first == 1)
  {
    // not all action components are guaranteed to inherit from ComponentMeshTransformerHelper,
    // which we need to specify the direction.
    const auto & first_component_cmth =
        _awh.getAction<ComponentMeshTransformHelper>(getParam<ComponentName>("first_component"));
    const auto & second_component_cmth =
        _awh.getAction<ComponentMeshTransformHelper>(getParam<ComponentName>("second_component"));

    // find start and end directions (may need to take the negative of the end direction).
    // obey user parameters if set

    RealVectorValue start_direction, end_direction;
    if (isParamValid("first_direction"))
      start_direction = getParam<RealVectorValue>("first_direction");
    else
      start_direction = first_component_cmth.direction();
    if (isParamValid("second_direction"))
      end_direction = getParam<RealVectorValue>("second_direction");
    else
      end_direction = second_component_cmth.direction();

    InputParameters bspline_params = _factory.getValidParams("BSplineCurveGenerator");
    bspline_params.set<RealVectorValue>("start_direction") = start_direction;
    bspline_params.set<RealVectorValue>("end_direction") = -end_direction;
    bspline_params.set<unsigned int>("num_elements") = getParam<unsigned int>("n_elem_normal");
    if (isParamValid("sharpness"))
      bspline_params.set<Real>("sharpness") = getParam<Real>("sharpness");
    bspline_params.set<unsigned int>("num_cps") = getParam<unsigned int>("num_cps");
    bspline_params.set<MeshGeneratorName>("start_mesh") =
        first_component.mg_names()
            .back(); // get last name from list of generators creating the component
    bspline_params.set<MeshGeneratorName>("end_mesh") =
        second_component.mg_names()
            .back(); // get last name from list of generators creating the component
    bspline_params.set<BoundaryName>("start_boundary") = getParam<BoundaryName>("first_boundary");
    bspline_params.set<BoundaryName>("end_boundary") = getParam<BoundaryName>("second_boundary");
    bspline_params.set<std::vector<BoundaryName>>("edge_nodesets") = {
        name() + "_bspline_start_node", name() + "_bspline_end_node"};
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "BSplineCurveGenerator", name() + "_curve", bspline_params);
    _mg_names.push_back(name() + "_curve");

    // stitcher for results
    if (getParam<ComponentName>("first_component") != getParam<ComponentName>("second_component"))
    {
      InputParameters stitcher_params = _factory.getValidParams("StitchMeshGenerator");
      stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") =
          std::vector<MeshGeneratorName>{first_component.mg_names().back(),
                                         name() + "_curve",
                                         second_component.mg_names().back()};
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
      mesh_stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
          {first_boundary, name() + "_bspline_start_node"}};
      mesh_stitcher_params.set<bool>("verbose_stitching") = _verbose;
      mesh_stitcher_params.set<bool>("output") = _verbose;
      mesh_stitcher_params.set<bool>("enforce_all_nodes_match_on_boundaries") =
          _enforce_all_nodes_match_on_boundaries;
      _app.getMeshGeneratorSystem().addMeshGenerator(
          "StitchMeshGenerator", name() + "_mesh_stitcher", mesh_stitcher_params);
      _mg_names.push_back(name() + "_mesh_stitcher");

      InputParameters boundary_stitcher_params =
          _factory.getValidParams("StitchBoundaryMeshGenerator");
      boundary_stitcher_params.set<MeshGeneratorName>("input") = name() + "_mesh_stitcher";
      boundary_stitcher_params.set<std::vector<std::vector<std::string>>>(
          "stitch_boundaries_pairs") = {{name() + "_bspline_end_node", second_boundary}};
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