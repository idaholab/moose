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

  MooseEnum junction_type("stitch_meshes fill_gap", "fill_gap");
  params.addParam<MooseEnum>("junction_method", junction_type, "How to join the two components");

  // Parameters for changing radius -- final radius will be calculated under the hood
  MooseEnum radial_growth_methods("LINEAR CUBIC", "LINEAR");
  params.addParam<MooseEnum>("radial_growth_method",
                             radial_growth_methods,
                             "Functional form to change radius while extruding along curve.");
  params.addParam<Real>("start_radial_growth_rate", 0, "Starting rate of radial expansion.");
  params.addParam<Real>("end_radial_growth_rate", 0, "Ending rate of radial expansion.");
  params.addRangeCheckedParam<libMesh::Real>(
      "r_final",
      "r_final>0.0",
      "Final radius to extrude to. Defaults to constant radius if not specified.");

  // Parameters for the region between meshes
  params.addRequiredParam<unsigned int>(
      "n_elem_normal", "Number of elements in the normal direction of the junction");
  params.addParam<SubdomainName>("block", "Block name for the junction, if a block is created.");

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
  // params.addRequiredRangeCheckedParam<unsigned int>(
  //     "num_elements", "num_elements>=1", "Numer of elements to be drawn. Must be at least 1.");

  // add parameters to an Advanced group for additional controls
  params.addParamNamesToGroup("sharpness num_cps edge_element_type r_final radial_growth_method "
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
    _junction_method(getParam<MooseEnum>("junction_method"))
{
  addRequiredTask("add_mesh_generator");

  // Check parameters
  // if (!_junction_method.contains("fill_gap"))
  //   errorDependentParameter("junction_method", "fill_gap_and_stitch", {"n_elem_normal",
  //   "block"});
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

  std::cout << "First Component boundary  = " << first_boundary << std::endl;
  std::cout << "Second Component boundary = " << second_boundary << std::endl;

  // Get the dimension of the components
  const auto dimension_first = first_component.dimension();
  const auto dimension_second = second_component.dimension();

  std::cout << "First Component dimension = " << dimension_first << std::endl;
  std::cout << "Second Component dimension = " << dimension_second << std::endl;

  if (dimension_first == 0 || dimension_second == 0)
    mooseError("Connecting 0 dimension meshes not implemented!");

  // Perform junction
  if (_junction_method == "stitch_meshes")
  {
    // Fairly easy to stitch this
    if (dimension_first == dimension_second)
    {
      // Stitch the two meshes
      InputParameters params = _factory.getValidParams("StitchedMeshGenerator");
      params.set<std::vector<MeshGeneratorName>>("inputs") =
          _mg_names; // protected member of ActionComponent
      params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
          {first_boundary, second_boundary}};
    }
    else
      mooseError("Stiching meshes of different dimensions is not implemented");
  }
  else if (_junction_method == "fill_gap")
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

    // find start and end directions (may need to take the negative of the end direction)
    RealVectorValue start_direction = first_component_cmth.direction();
    RealVectorValue end_direction = second_component_cmth.direction();

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
    InputParameters ld_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
    ld_params.set<MeshGeneratorName>("input") = first_component.mg_names().back();
    ld_params.set<std::vector<BoundaryName>>("sidesets") =
        std::vector{getParam<BoundaryName>("first_boundary")};
    ld_params.set<SubdomainName>("new_block_name") = (SubdomainName)(name() + "_LowerDBlock");
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "LowerDBlockFromSidesetGenerator", name() + "_lowerDGeneration", ld_params);
    _mg_names.push_back(name() + "_lowerDGeneration");

    InputParameters bmc_params = _factory.getValidParams("BlockToMeshConverterGenerator");
    bmc_params.set<MeshGeneratorName>("input") = name() + "_lowerDGeneration";
    bmc_params.set<std::vector<SubdomainName>>("target_blocks") = {
        (SubdomainName)(name() + "_LowerDBlock")};
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "BlockToMeshConverterGenerator", name() + "_blockToMesh", bmc_params);
    _mg_names.push_back(name() + "_blockToMesh");

    // set up AdvancedExtruderGenerator
    InputParameters aeg_params = _factory.getValidParams("AdvancedExtruderGenerator");
    aeg_params.set<MeshGeneratorName>("extrusion_curve") = (MeshGeneratorName)(name() + "_curve");
    aeg_params.set<MeshGeneratorName>("input") = (MeshGeneratorName)(name() + "_blockToMesh");

    aeg_params.set<Point>("start_extrusion_direction") = start_direction;
    aeg_params.set<Point>("end_extrusion_direction") = end_direction;
    if (isParamValid("r_final"))
    {
      // this will be conditionally set... hopefully r_final can be automated in the future
      aeg_params.set<Real>("r_final") = getParam<Real>("r_final");
      aeg_params.set<Real>("start_radial_growth_rate") = getParam<Real>("start_radial_growth_rate");
      aeg_params.set<Real>("end_radial_growth_rate") = getParam<Real>("end_radial_growth_rate");
      aeg_params.set<MooseEnum>("radial_growth_method") =
          getParam<MooseEnum>("radial_growth_method");
    }
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "AdvancedExtruderGenerator", name() + "_base", aeg_params);
    _mg_names.push_back(name() + "_base");

    // stitcher for results
    InputParameters stitcher_params = _factory.getValidParams("StitchedMeshGenerator");
    stitcher_params.set<std::vector<MeshGeneratorName>>("inputs") = _mg_names;
    stitcher_params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
        {first_boundary, second_boundary}};
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