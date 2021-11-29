//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadiationTransferAction.h"
#include "Factory.h"
#include "MooseMesh.h"
#include "MeshGeneratorMesh.h"
#include "FEProblemBase.h"
#include "PatchSidesetGenerator.h"
#include "ViewFactorRayBC.h"
#include "ViewFactorRayStudy.h"

registerMooseAction("HeatConductionApp", RadiationTransferAction, "append_mesh_generator");
registerMooseAction("HeatConductionApp", RadiationTransferAction, "setup_mesh_complete");
registerMooseAction("HeatConductionApp", RadiationTransferAction, "add_user_object");
registerMooseAction("HeatConductionApp", RadiationTransferAction, "add_bc");
registerMooseAction("HeatConductionApp", RadiationTransferAction, "add_ray_boundary_condition");

InputParameters
RadiationTransferAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "This action sets up the net radiation calculation between specified sidesets.");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The boundaries that participate in the radiative exchange.");

  params.addParam<std::vector<BoundaryName>>(
      "adiabatic_boundary", "The adiabatic boundaries that participate in the radiative exchange.");

  params.addParam<std::vector<BoundaryName>>(
      "fixed_temperature_boundary",
      "The fixed temperature boundaries that participate in the radiative exchange.");

  params.addParam<std::vector<FunctionName>>("fixed_boundary_temperatures",
                                             "The temperatures of the fixed boundary.");

  params.addRequiredParam<std::vector<unsigned int>>("n_patches",
                                                     "Number of radiation patches per sideset.");
  MultiMooseEnum partitioning(
      "default=-3 metis=-2 parmetis=-1 linear=0 centroid hilbert_sfc morton_sfc", "default");
  partitioning += "grid";
  params.addParam<MultiMooseEnum>(
      "partitioners",
      partitioning,
      "Specifies a mesh partitioner to use when preparing the radiation patches.");

  MultiMooseEnum direction("x y z radial");
  params.addParam<MultiMooseEnum>("centroid_partitioner_directions",
                                  direction,
                                  "Specifies the sort direction if using the centroid partitioner. "
                                  "Available options: x, y, z, radial");

  params.addRequiredParam<VariableName>("temperature", "The coupled temperature variable.");
  params.addRequiredParam<std::vector<Real>>("emissivity", "Emissivities for each boundary.");

  MooseEnum view_factor_calculator("analytical ray_tracing", "ray_tracing");
  params.addParam<MooseEnum>(
      "view_factor_calculator", view_factor_calculator, "The view factor calculator being used.");

  params.addParam<bool>(
      "print_view_factor_info", false, "Flag to print information about computed view factors.");
  params.addParam<bool>("normalize_view_factor",
                        true,
                        "Determines if view factors are normalized to sum to one (consistent with "
                        "their definition).");

  std::vector<BoundaryName> empty = {};
  params.addParam<std::vector<BoundaryName>>(
      "symmetry_boundary",
      empty,
      "The sidesets that represent symmetry lines/planes for the problem. These sidesets do not "
      "participate in the radiative exchange"
      "so they should not be listed in the sidesets parameter.");

  MooseEnum qtypes("GAUSS GRID", "GRID");
  params.addParam<MooseEnum>(
      "ray_tracing_face_type", qtypes, "The face quadrature rule type used for ray tracing.");

  MooseEnum qorders("CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH "
                    "ELEVENTH TWELFTH THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH "
                    "EIGHTTEENTH NINTEENTH TWENTIETH",
                    "CONSTANT");
  params.addParam<MooseEnum>(
      "ray_tracing_face_order", qorders, "The face quadrature rule order used for ray tracing.");

  params.addParam<unsigned int>(
      "polar_quad_order",
      16,
      "Order of the polar quadrature [polar angle is between ray and normal]. Must be even. Only "
      "used if view_factor_calculator = ray_tracing.");
  params.addParam<unsigned int>(
      "azimuthal_quad_order",
      8,
      "Order of the azimuthal quadrature per quadrant [azimuthal angle is measured in "
      "a plane perpendicular to the normal]. Only used if view_factor_calculator = "
      "ray_tracing.");

  return params;
}

RadiationTransferAction::RadiationTransferAction(const InputParameters & params)
  : Action(params),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
    _view_factor_calculator(getParam<MooseEnum>("view_factor_calculator"))
{
  const auto & symmetry_names = getParam<std::vector<BoundaryName>>("symmetry_boundary");

  if (_view_factor_calculator != "ray_tracing")
  {
    for (const auto & param_name : {"polar_quad_order",
                                    "azimuthal_quad_order",
                                    "ray_tracing_face_type",
                                    "ray_tracing_face_order"})
      if (params.isParamSetByUser(param_name))
        paramWarning(param_name,
                     "Only used for view_factor_calculator = ray_tracing. It is ignored for this "
                     "calculation.");

    if (symmetry_names.size())
      paramError("symmetry_boundary",
                 "Symmetry boundaries are only supported with view_factor_calculator = "
                 "ray_tracing.");
  }
  else
  {
    // check that there is no overlap between sidesets and symmetry sidesets
    for (const auto & name : _boundary_names)
      if (std::find(symmetry_names.begin(), symmetry_names.end(), name) != symmetry_names.end())
        paramError("boundary",
                   "Boundary ",
                   name,
                   " is present in parameter boundary and symmetry_boundary.");
  }
}

void
RadiationTransferAction::act()
{
  if (_current_task == "append_mesh_generator")
    addMeshGenerator();
  else if (_current_task == "setup_mesh_complete")
    radiationPatchNames();
  else if (_current_task == "add_user_object")
  {
    addRadiationObject();
    addRayStudyObject();
    addViewFactorObject();
  }
  else if (_current_task == "add_bc")
    addRadiationBCs();
  else if (_current_task == "add_ray_boundary_condition")
    addRayBCs();
}

void
RadiationTransferAction::addRadiationBCs() const
{
  InputParameters params = _factory.getValidParams("GrayLambertNeumannBC");

  // set boundary
  std::vector<std::vector<std::string>> radiation_patch_names = bcRadiationPatchNames();
  std::vector<BoundaryName> boundary_names;
  for (auto & e1 : radiation_patch_names)
    for (auto & e2 : e1)
      boundary_names.push_back(e2);

  params.set<std::vector<BoundaryName>>("boundary") = boundary_names;

  // set temperature variable
  params.set<NonlinearVariableName>("variable") = getParam<VariableName>("temperature");

  // set radiationuserobject
  params.set<UserObjectName>("surface_radiation_object_name") = radiationObjectName();

  _problem->addBoundaryCondition("GrayLambertNeumannBC", "gray_lamber_neumann_bc_" + _name, params);
}

void
RadiationTransferAction::addViewFactorObject() const
{
  std::vector<std::vector<std::string>> radiation_patch_names = radiationPatchNames();
  std::vector<BoundaryName> boundary_names;
  for (auto & e1 : radiation_patch_names)
    for (auto & e2 : e1)
      boundary_names.push_back(e2);

  // this userobject is only executed on initial
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL};

  if (_view_factor_calculator == "analytical")
  {
    // this branch adds the UnobstructedPlanarViewFactor
    InputParameters params = _factory.getValidParams("UnobstructedPlanarViewFactor");
    params.set<std::vector<BoundaryName>>("boundary") = boundary_names;
    params.set<ExecFlagEnum>("execute_on") = exec_enum;

    _problem->addUserObject("UnobstructedPlanarViewFactor", viewFactorObjectName(), params);
  }
  else if (_view_factor_calculator == "ray_tracing")
  {
    // this branch adds the ray tracing UO
    InputParameters params = _factory.getValidParams("RayTracingViewFactor");
    params.set<std::vector<BoundaryName>>("boundary") = boundary_names;
    params.set<ExecFlagEnum>("execute_on") = exec_enum;
    params.set<UserObjectName>("ray_study_name") = rayStudyName();
    params.set<bool>("print_view_factor_info") = getParam<bool>("print_view_factor_info");
    params.set<bool>("normalize_view_factor") = getParam<bool>("normalize_view_factor");
    _problem->addUserObject("RayTracingViewFactor", viewFactorObjectName(), params);
  }
}

void
RadiationTransferAction::addRayStudyObject() const
{
  if (_view_factor_calculator == "analytical")
    return;

  std::vector<std::vector<std::string>> radiation_patch_names = radiationPatchNames();
  std::vector<BoundaryName> boundary_names;
  for (auto & e1 : radiation_patch_names)
    for (auto & e2 : e1)
      boundary_names.push_back(e2);

  InputParameters params = _factory.getValidParams("ViewFactorRayStudy");

  params.set<std::vector<BoundaryName>>("boundary") = boundary_names;

  // set this object to be execute on initial only
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL};
  params.set<ExecFlagEnum>("execute_on") = exec_enum;

  // set face order
  params.set<MooseEnum>("face_order") = getParam<MooseEnum>("ray_tracing_face_order");
  params.set<MooseEnum>("face_type") = getParam<MooseEnum>("ray_tracing_face_type");

  // set angular quadrature
  params.set<unsigned int>("polar_quad_order") = getParam<unsigned int>("polar_quad_order");
  params.set<unsigned int>("azimuthal_quad_order") = getParam<unsigned int>("azimuthal_quad_order");
  _problem->addUserObject("ViewFactorRayStudy", rayStudyName(), params);
}

void
RadiationTransferAction::addRayBCs() const
{
  if (_view_factor_calculator == "analytical")
    return;

  std::vector<std::vector<std::string>> radiation_patch_names = radiationPatchNames();
  std::vector<BoundaryName> boundary_names;
  for (auto & e1 : radiation_patch_names)
    for (auto & e2 : e1)
      boundary_names.push_back(e2);

  {
    InputParameters params = _factory.getValidParams("ViewFactorRayBC");
    params.set<std::vector<BoundaryName>>("boundary") = boundary_names;
    params.set<RayTracingStudy *>("_ray_tracing_study") =
        &_problem->getUserObject<ViewFactorRayStudy>(rayStudyName());
    _problem->addObject<RayBoundaryConditionBase>("ViewFactorRayBC", rayBCName(), params);
  }

  // add symmetry BCs if applicable
  const auto & symmetry_names = getParam<std::vector<BoundaryName>>("symmetry_boundary");
  if (symmetry_names.size() > 0)
  {
    InputParameters params = _factory.getValidParams("ReflectRayBC");
    params.set<std::vector<BoundaryName>>("boundary") = symmetry_names;
    params.set<RayTracingStudy *>("_ray_tracing_study") =
        &_problem->getUserObject<ViewFactorRayStudy>(rayStudyName());
    _problem->addObject<RayBoundaryConditionBase>("ReflectRayBC", symmetryRayBCName(), params);
  }
}

UserObjectName
RadiationTransferAction::rayStudyName() const
{
  return "ray_study_uo_" + _name;
}

std::string
RadiationTransferAction::rayBCName() const
{
  return "ray_bc_" + _name;
}

std::string
RadiationTransferAction::symmetryRayBCName() const
{
  return "symmetry_ray_bc_" + _name;
}

UserObjectName
RadiationTransferAction::viewFactorObjectName() const
{
  return "view_factor_uo_" + _name;
}

UserObjectName
RadiationTransferAction::radiationObjectName() const
{
  return "view_factor_surface_radiation_" + _name;
}

void
RadiationTransferAction::addRadiationObject() const
{
  std::vector<std::vector<std::string>> radiation_patch_names = radiationPatchNames();

  // input parameter check
  std::vector<Real> emissivity = getParam<std::vector<Real>>("emissivity");
  if (emissivity.size() != _boundary_names.size())
    mooseError("emissivity parameter needs to be the same size as the boundary parameter.");

  // the action only sets up ViewFactorObjectSurfaceRadiation, because after splitting
  // faces auotmatically, it makes no sense to require view factor input by hand.
  InputParameters params = _factory.getValidParams("ViewFactorObjectSurfaceRadiation");
  params.set<std::vector<VariableName>>("temperature") = {getParam<VariableName>("temperature")};

  std::vector<Real> extended_emissivity;
  for (unsigned int j = 0; j < _boundary_names.size(); ++j)
    for (unsigned int i = 0; i < nPatch(j); ++i)
      extended_emissivity.push_back(emissivity[j]);
  params.set<std::vector<Real>>("emissivity") = extended_emissivity;

  // add boundary parameter
  std::vector<BoundaryName> boundary_names;
  for (auto & e1 : radiation_patch_names)
    for (auto & e2 : e1)
      boundary_names.push_back(e2);
  params.set<std::vector<BoundaryName>>("boundary") = boundary_names;

  // add adiabatic_boundary parameter if required
  if (isParamValid("adiabatic_boundary"))
  {
    std::vector<BoundaryName> adiabatic_boundary_names =
        getParam<std::vector<BoundaryName>>("adiabatic_boundary");
    std::vector<BoundaryName> adiabatic_patch_names;
    for (unsigned int k = 0; k < adiabatic_boundary_names.size(); ++k)
    {
      BoundaryName bnd_name = adiabatic_boundary_names[k];

      // find the right entry in _boundary_names
      auto it = std::find(_boundary_names.begin(), _boundary_names.end(), bnd_name);

      // check if entry was found: it must be found or an error would occur later
      if (it == _boundary_names.end())
        mooseError("Adiabatic boundary ", bnd_name, " not present in boundary.");

      // this is the position in the _boundary_names vector; this is what
      // we are really after
      auto index = std::distance(_boundary_names.begin(), it);

      // collect the correct boundary names
      for (auto & e : radiation_patch_names[index])
        adiabatic_patch_names.push_back(e);
    }
    params.set<std::vector<BoundaryName>>("adiabatic_boundary") = adiabatic_patch_names;
  }

  // add isothermal sidesets if required
  if (isParamValid("fixed_temperature_boundary"))
  {
    if (!isParamValid("fixed_boundary_temperatures"))
      mooseError("fixed_temperature_boundary is provided so fixed_boundary_temperatures must be "
                 "provided too");

    std::vector<BoundaryName> fixed_T_boundary_names =
        getParam<std::vector<BoundaryName>>("fixed_temperature_boundary");

    std::vector<FunctionName> fixed_T_funcs =
        getParam<std::vector<FunctionName>>("fixed_boundary_temperatures");

    // check length of fixed_boundary_temperatures
    if (fixed_T_funcs.size() != fixed_T_boundary_names.size())
      mooseError("Size of parameter fixed_boundary_temperatures and fixed_temperature_boundary "
                 "must be equal.");

    std::vector<BoundaryName> fixed_T_patch_names;
    std::vector<FunctionName> fixed_T_function_names;
    for (unsigned int k = 0; k < fixed_T_boundary_names.size(); ++k)
    {
      BoundaryName bnd_name = fixed_T_boundary_names[k];

      // find the right entry in _boundary_names
      auto it = std::find(_boundary_names.begin(), _boundary_names.end(), bnd_name);

      // check if entry was found: it must be found or an error would occur later
      if (it == _boundary_names.end())
        mooseError("Fixed temperature sideset ", bnd_name, " not present in boundary.");

      // this is the position in the _boundary_names vector; this is what
      // we are really after
      auto index = std::distance(_boundary_names.begin(), it);

      // collect the correct boundary names
      for (auto & e : radiation_patch_names[index])
      {
        fixed_T_patch_names.push_back(e);
        fixed_T_function_names.push_back(fixed_T_funcs[k]);
      }
    }
    params.set<std::vector<BoundaryName>>("fixed_temperature_boundary") = fixed_T_patch_names;
    params.set<std::vector<FunctionName>>("fixed_boundary_temperatures") = fixed_T_function_names;
  }

  // the view factor userobject name
  params.set<UserObjectName>("view_factor_object_name") = viewFactorObjectName();

  // this userobject needs to be executed on linear and timestep end
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_LINEAR, EXEC_TIMESTEP_END};
  params.set<ExecFlagEnum>("execute_on") = exec_enum;

  // add the object
  _problem->addUserObject("ViewFactorObjectSurfaceRadiation", radiationObjectName(), params);
}

std::vector<std::vector<std::string>>
RadiationTransferAction::radiationPatchNames() const
{
  std::vector<std::vector<std::string>> radiation_patch_names(_boundary_names.size());
  std::vector<BoundaryID> boundary_ids = _mesh->getBoundaryIDs(_boundary_names);
  for (unsigned int j = 0; j < boundary_ids.size(); ++j)
  {
    boundary_id_type bid = boundary_ids[j];
    std::string base_name = _mesh->getBoundaryName(bid);
    std::vector<std::string> bnames;
    for (unsigned int i = 0; i < nPatch(j); ++i)
    {
      std::stringstream ss;
      ss << base_name << "_" << i;
      bnames.push_back(ss.str());
    }
    radiation_patch_names[j] = bnames;
  }
  return radiation_patch_names;
}

std::vector<std::vector<std::string>>
RadiationTransferAction::bcRadiationPatchNames() const
{
  auto ad_bnd_names = getParam<std::vector<BoundaryName>>("adiabatic_boundary");
  auto ft_bnd_names = getParam<std::vector<BoundaryName>>("fixed_temperature_boundary");
  std::vector<std::vector<std::string>> radiation_patch_names;
  std::vector<BoundaryID> boundary_ids = _mesh->getBoundaryIDs(_boundary_names);
  for (unsigned int j = 0; j < boundary_ids.size(); ++j)
  {
    boundary_id_type bid = boundary_ids[j];
    BoundaryName bnd_name = _boundary_names[j];

    // check if this sideset is adiabatic or isothermal
    auto it_a = std::find(ad_bnd_names.begin(), ad_bnd_names.end(), bnd_name);
    auto it_t = std::find(ft_bnd_names.begin(), ft_bnd_names.end(), bnd_name);
    if (it_a != ad_bnd_names.end() || it_t != ft_bnd_names.end())
      continue;

    std::string base_name = _mesh->getBoundaryName(bid);
    std::vector<std::string> bnames;
    for (unsigned int i = 0; i < nPatch(j); ++i)
    {
      std::stringstream ss;
      ss << base_name << "_" << i;
      bnames.push_back(ss.str());
    }
    radiation_patch_names.push_back(bnames);
  }
  return radiation_patch_names;
}

void
RadiationTransferAction::addMeshGenerator()
{
  std::vector<unsigned int> n_patches = getParam<std::vector<unsigned int>>("n_patches");
  MultiMooseEnum partitioners = getParam<MultiMooseEnum>("partitioners");
  if (!_pars.isParamSetByUser("partitioners"))
  {
    partitioners.clear();
    for (unsigned int j = 0; j < _boundary_names.size(); ++j)
      partitioners.push_back("metis");
  }

  MultiMooseEnum direction = getParam<MultiMooseEnum>("centroid_partitioner_directions");

  // check input parameters
  if (_boundary_names.size() != n_patches.size())
    mooseError("n_patches parameter must have same length as boundary parameter.");

  if (_boundary_names.size() != partitioners.size())
    mooseError("partitioners parameter must have same length as boundary parameter.");

  for (unsigned int j = 0; j < partitioners.size(); ++j)
    if (partitioners[j] == "centroid" && direction.size() != _boundary_names.size())
      mooseError(
          "centroid partitioner is selected for at least one sideset. "
          "centroid_partitioner_directions parameter must have same length as boundary parameter.");

  // check if mesh is a MeshGeneratorMesh
  std::shared_ptr<MeshGeneratorMesh> mg_mesh = std::dynamic_pointer_cast<MeshGeneratorMesh>(_mesh);
  if (!mg_mesh)
    mooseError("This action adds MeshGenerator objects and therefore only works with a "
               "MeshGeneratorMesh.");

  for (unsigned int j = 0; j < _boundary_names.size(); ++j)
  {
    InputParameters params = _factory.getValidParams("PatchSidesetGenerator");
    params.set<BoundaryName>("boundary") = _boundary_names[j];
    params.set<unsigned int>("n_patches") = n_patches[j];
    params.set<MooseEnum>("partitioner") = partitioners[j];

    if (partitioners[j] == "centroid")
      params.set<MooseEnum>("centroid_partitioner_direction") = direction[j];

    _app.appendMeshGenerator("PatchSidesetGenerator", meshGeneratorName(j), params);
  }
}

unsigned int
RadiationTransferAction::nPatch(unsigned int j) const
{
  const MeshGenerator * mg = &_app.getMeshGenerator(meshGeneratorName(j));
  const PatchSidesetGenerator * psg = dynamic_cast<const PatchSidesetGenerator *>(mg);
  if (!psg)
    mooseError("Failed to convert mesh generator ", mg->name(), " to PatchSidesetGenerator.");
  return psg->nPatches();
}

MeshGeneratorName
RadiationTransferAction::meshGeneratorName(unsigned int j) const
{
  std::stringstream ss;
  ss << "patch_side_set_generator_" << _boundary_names[j];
  return ss.str();
}
