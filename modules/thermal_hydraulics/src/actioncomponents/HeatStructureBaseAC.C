//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureBaseAC.h"
#include "ActionFactory.h"

InputParameters
HeatStructureBaseAC::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += ComponentMeshTransformHelper::validParams();
  params.addClassDescription(
      "Base class for 2D heat-conducting solid ActionComponents, with a mesh generated through "
      "the MeshGenerator system.");

  params.addRequiredParam<std::vector<std::string>>("names", "Name of each transverse region");
  params.addRequiredParam<std::vector<Real>>("widths", "Width of each transverse region [m]");
  params.addRequiredParam<std::vector<unsigned int>>(
      "n_part_elems", "Number of elements of each transverse region");
  params.addParam<std::vector<UserObjectName>>(
      "solid_properties", "Solid properties object name for each transverse region");
  params.addParam<std::vector<Real>>(
      "solid_properties_T_ref",
      {},
      "Density reference temperatures for each transverse region. This is required if "
      "'solid_properties' is provided. The density in each region will be a constant value "
      "computed by evaluating the density function at the reference temperature.");
  params.addParam<Real>("num_rods", 1.0, "Number of rods represented by this heat structure");

  params.addRequiredParam<FunctionName>("initial_T", "Initial temperature [K]");
  params.addParam<Real>(
      "scaling_factor_temperature", 1.0, "Scaling factor for solid temperature variable.");

  params.addRequiredRangeCheckedParam<Real>("length", "length > 0", "Length of the heat structure");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_elems", "n_elems > 0", "Number of elements along the length of the heat structure");

  params.renameParam("direction",
                     "orientation",
                     "Direction to orient the heat structure with, assuming it is initially "
                     "oriented along the X-axis (1, 0, 0).");

  return params;
}

HeatStructureBaseAC::HeatStructureBaseAC(const InputParameters & params)
  : ActionComponent(params),
    ComponentMeshTransformHelper(params),
    _region_names(getParam<std::vector<std::string>>("names")),
    _widths(getParam<std::vector<Real>>("widths")),
    _n_part_elems(getParam<std::vector<unsigned int>>("n_part_elems")),
    _length(getParam<Real>("length")),
    _n_elems(getParam<unsigned int>("n_elems")),
    _physics(nullptr)
{
  if (_widths.size() != _region_names.size() || _n_part_elems.size() != _region_names.size())
    paramError("names", "'names', 'widths' and 'n_part_elems' must all be the same size.");
  if (isParamValid("solid_properties"))
  {
    const auto sp_names = getParam<std::vector<UserObjectName>>("solid_properties");
    const auto T_ref = getParam<std::vector<Real>>("solid_properties_T_ref");
    if (sp_names.size() != _region_names.size() || T_ref.size() != _region_names.size())
      paramError("solid_properties",
                 "'solid_properties' and 'solid_properties_T_ref' must have as many entries as "
                 "'names'.");
  }

  _dimension = 2;
  for (const auto & region_name : _region_names)
    _blocks.push_back(genName(name(), region_name));

  addRequiredTask("add_mesh_generator");
  addRequiredTask("init_component_physics");
  addRequiredTask("add_material");

  // Build and register the physics defining this heat structure's governing equation, mirroring
  // how FlowChannel1PhaseAC builds its own FlowChannel1PhasePhysics - so the user does not need to
  // declare a separate [Physics] block.
  const std::string physics_type = "HeatStructurePhysics";
  InputParameters physics_params = _action_factory.getValidParams(physics_type);
  physics_params.blockFullpath() = params.blockFullpath();
  physics_params.set<bool>("_built_by_moose") = true;
  physics_params.set<std::string>("registered_identifier") = "(AutoBuilt)";

  physics_params.set<VariableName>("temperature_name") = "T_solid";
  physics_params.set<FunctionName>("initial_temperature") = getParam<FunctionName>("initial_T");
  physics_params.set<Real>("scaling_factor_temperature") =
      getParam<Real>("scaling_factor_temperature");
  // HeatConductionPhysicsBase::addPreconditioning() otherwise unconditionally injects '-pc_type
  // hypre -pc_hypre_type boomeramg' into the PETSc options, silently overriding whatever the
  // simulation's own [Preconditioning] block requests. Deferring keeps preconditioning entirely
  // under the user's/executioner's control, matching how FlowChannel1PhasePhysics never touches
  // preconditioning either.
  physics_params.set<MooseEnum>("preconditioning") = "defer";

  auto physics_action = _action_factory.create(physics_type, name() + "_physics", physics_params);
  _physics = dynamic_cast<HeatStructurePhysics *>(physics_action.get());
  if (!_physics)
    mooseError("Internal error: failed to create this component's HeatStructurePhysics");
  _awh.addActionBlock(physics_action);
}

void
HeatStructureBaseAC::addMeshGenerators()
{
  std::vector<MeshGeneratorName> region_mg_names;
  for (unsigned int i = 0; i < _region_names.size(); i++)
  {
    const std::string base_name = genSafeName(name(), _region_names[i], "base");
    InputParameters params = _factory.getValidParams("GeneratedMeshGenerator");
    params.set<MooseEnum>("dim") = "2";
    params.set<unsigned int>("nx") = _n_elems;
    params.set<unsigned int>("ny") = _n_part_elems[i];
    params.set<Real>("xmax") = _length;
    params.set<Real>("ymax") = _widths[i];
    // StackGenerator (unlike CombinerGenerator) does not disambiguate colliding subdomain ids when
    // stitching meshes together, and GeneratedMeshGenerator defaults every region to id 0 - so an
    // explicit, unique id per region is required here.
    params.set<std::vector<SubdomainID>>("subdomain_ids") = {static_cast<SubdomainID>(i)};
    params.set<SubdomainName>("subdomain_name") = _blocks[i];
    _app.getMeshGeneratorSystem().addMeshGenerator("GeneratedMeshGenerator", base_name, params);
    _mg_names.push_back(base_name);
    region_mg_names.push_back(base_name);
  }

  // Stack the regions along the transverse ('y') direction. All regions were left with their
  // default 'top'/'bottom' boundary names, so StackGenerator can stitch consecutive regions
  // together automatically; the outermost region's 'top' and innermost region's 'bottom' survive
  // un-stitched. StackGenerator requires at least 2 inputs (it indexes into a "rest of the inputs"
  // vector unconditionally), so a single region is instead just translated by the same offset.
  std::string stack_name;
  if (region_mg_names.size() > 1)
  {
    stack_name = genSafeName(name(), "stack");
    InputParameters params = _factory.getValidParams("StackGenerator");
    params.set<MooseEnum>("dim") = "2";
    params.set<std::vector<MeshGeneratorName>>("inputs") = region_mg_names;
    params.set<Real>("bottom_height") = yOffset();
    _app.getMeshGeneratorSystem().addMeshGenerator("StackGenerator", stack_name, params);
  }
  else
  {
    stack_name = genSafeName(name(), "stack");
    InputParameters params = _factory.getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = region_mg_names[0];
    params.set<MooseEnum>("transform") = "TRANSLATE";
    params.set<RealVectorValue>("vector_value") = RealVectorValue(0, yOffset(), 0);
    _app.getMeshGeneratorSystem().addMeshGenerator("TransformGenerator", stack_name, params);
  }
  _mg_names.push_back(stack_name);

  // stitch_meshes (used by StackGenerator) preserves each region's numeric subdomain id (made
  // unique above) but does not carry over their subdomain *names* from the meshes it consumes -
  // only the primary (first) region's name survives. Re-establish every region's name by its
  // now-guaranteed-unique numeric id.
  const std::string block_renamed_name = genSafeName(name(), "block_names");
  {
    std::vector<SubdomainName> old_block;
    for (unsigned int i = 0; i < _region_names.size(); i++)
      old_block.push_back(std::to_string(i));

    InputParameters params = _factory.getValidParams("RenameBlockGenerator");
    params.set<MeshGeneratorName>("input") = stack_name;
    params.set<std::vector<SubdomainName>>("old_block") = old_block;
    params.set<std::vector<SubdomainName>>("new_block") = _blocks;
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "RenameBlockGenerator", block_renamed_name, params);
    _mg_names.push_back(block_renamed_name);
  }

  // Rename the surviving 'top'/'bottom' boundaries to the aggregate inner/outer names.
  const std::string renamed_name = genSafeName(name(), "outer_inner");
  {
    InputParameters params = _factory.getValidParams("RenameBoundaryGenerator");
    params.set<MeshGeneratorName>("input") = block_renamed_name;
    params.set<std::vector<BoundaryName>>("old_boundary") = {"top", "bottom"};
    params.set<std::vector<BoundaryName>>("new_boundary") = {genName(name(), "outer"),
                                                             genName(name(), "inner")};
    _app.getMeshGeneratorSystem().addMeshGenerator("RenameBoundaryGenerator", renamed_name, params);
    _mg_names.push_back(renamed_name);
  }

  // Tag the start/end faces, per region and in aggregate (spanning every region), directly from
  // geometry on the now-stacked mesh - SideSetsAroundSubdomainGenerator before stacking would only
  // have survived for the primary (first) region, the same way subdomain names did not survive
  // for the others (see the RenameBlockGenerator step above).
  std::string current = renamed_name;
  std::vector<std::pair<std::string, std::vector<SubdomainName>>> start_end_blocks;
  for (unsigned int i = 0; i < _region_names.size(); i++)
    start_end_blocks.emplace_back(_region_names[i], std::vector<SubdomainName>{_blocks[i]});
  start_end_blocks.emplace_back("", _blocks);
  for (const auto & name_and_blocks : start_end_blocks)
    for (const auto & side_normal :
         {std::make_pair(std::string("start"), RealVectorValue(-1, 0, 0)),
          std::make_pair(std::string("end"), RealVectorValue(1, 0, 0))})
    {
      const std::string step_name = genSafeName(
          name(), name_and_blocks.first.empty() ? "agg" : name_and_blocks.first, side_normal.first);
      InputParameters params = _factory.getValidParams("SideSetsAroundSubdomainGenerator");
      params.set<MeshGeneratorName>("input") = current;
      params.set<std::vector<SubdomainName>>("block") = name_and_blocks.second;
      params.set<Point>("normal") = side_normal.second;
      params.set<std::vector<BoundaryName>>("new_boundary") = {
          name_and_blocks.first.empty()
              ? genName(name(), side_normal.first)
              : genName(name(), name_and_blocks.first, side_normal.first)};
      _app.getMeshGeneratorSystem().addMeshGenerator(
          "SideSetsAroundSubdomainGenerator", step_name, params);
      _mg_names.push_back(step_name);
      current = step_name;
    }

  // Tag the interior interfaces between adjacent regions.
  for (unsigned int i = 0; i + 1 < _region_names.size(); i++)
  {
    const std::string step_name = genSafeName(name(), "interface", std::to_string(i));
    InputParameters params = _factory.getValidParams("SideSetsBetweenSubdomainsGenerator");
    params.set<MeshGeneratorName>("input") = current;
    params.set<std::vector<SubdomainName>>("primary_block") = {_blocks[i]};
    params.set<std::vector<SubdomainName>>("paired_block") = {_blocks[i + 1]};
    params.set<std::vector<BoundaryName>>("new_boundary") = {
        genName(name(), _region_names[i], _region_names[i + 1])};
    _app.getMeshGeneratorSystem().addMeshGenerator(
        "SideSetsBetweenSubdomainsGenerator", step_name, params);
    _mg_names.push_back(step_name);
    current = step_name;
  }

  // Every region's own default 'left'/'right' boundary tags (never renamed - see above) are left
  // dangling once the aggregate/per-region start/end boundaries have been derived from geometry.
  // They are redundant (each spans the same faces as one of the boundaries just derived) and,
  // being unprefixed, collide by name with every other heat structure's own leftover 'left'/
  // 'right' tags once multiple instances are combined into one simulation - so delete them here.
  {
    const std::string step_name = genSafeName(name(), "cleanup");
    InputParameters params = _factory.getValidParams("BoundaryDeletionGenerator");
    params.set<MeshGeneratorName>("input") = current;
    params.set<std::vector<BoundaryName>>("boundary_names") = {"left", "right"};
    _app.getMeshGeneratorSystem().addMeshGenerator("BoundaryDeletionGenerator", step_name, params);
    _mg_names.push_back(step_name);
    current = step_name;
  }

  _top_mg_name = current;

  ComponentMeshTransformHelper::addMeshGenerators();
}

void
HeatStructureBaseAC::addConstantDensitySolidPropertiesMaterial(const UserObjectName & sp_name,
                                                               Real T_ref,
                                                               const SubdomainName & block)
{
  const std::string class_name = "ADConstantDensityThermalSolidPropertiesMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = {block};
  params.set<std::vector<VariableName>>("temperature") = {"T_solid"};
  params.set<UserObjectName>("sp") = sp_name;
  params.set<Real>("T_ref") = T_ref;
  getProblem().addMaterial(class_name, genSafeName(name(), class_name, block), params);
}

void
HeatStructureBaseAC::addMaterials()
{
  if (!isParamValid("solid_properties"))
    return;

  const auto sp_names = getParam<std::vector<UserObjectName>>("solid_properties");
  const auto T_ref = getParam<std::vector<Real>>("solid_properties_T_ref");
  for (unsigned int i = 0; i < sp_names.size(); i++)
    addConstantDensitySolidPropertiesMaterial(sp_names[i], T_ref[i], _blocks[i]);
}

void
HeatStructureBaseAC::addPhysics()
{
  _physics->addComponent(*this);
}
