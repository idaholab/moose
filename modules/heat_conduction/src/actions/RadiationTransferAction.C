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

registerMooseAction("HeatConductionApp", RadiationTransferAction, "add_mesh_generator");
registerMooseAction("HeatConductionApp", RadiationTransferAction, "setup_mesh_complete");
registerMooseAction("HeatConductionApp", RadiationTransferAction, "add_user_object");
registerMooseAction("HeatConductionApp", RadiationTransferAction, "add_bc");

template <>
InputParameters
validParams<RadiationTransferAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription(
      "This action sets up the net radiation calculation between specified sidesets.");

  params.addRequiredParam<std::vector<boundary_id_type>>(
      "sidesets", "The sidesets that participate in the radiative exchange.");

  params.addParam<std::vector<boundary_id_type>>(
      "adiabatic_sidesets", "The adiabatic sidesets that participate in the radiative exchange.");

  params.addParam<std::vector<boundary_id_type>>(
      "fixed_temperature_sidesets",
      "The fixed temperature sidesets that participate in the radiative exchange.");

  params.addParam<std::vector<FunctionName>>("fixed_boundary_temperatures",
                                             "The temperatures of the fixed boundary.");

  params.addRequiredParam<std::vector<unsigned int>>("n_patches",
                                                     "Number of radiation patches per sideset.");
  MultiMooseEnum partitioning(
      "default=-3 metis=-2 parmetis=-1 linear=0 centroid hilbert_sfc morton_sfc", "default");
  params.addParam<MultiMooseEnum>(
      "partitioners",
      partitioning,
      "Specifies a mesh partitioner to use when preparing the radiation patches.");

  params.addRequiredParam<MeshGeneratorName>("final_mesh_generator",
                                             "Name of the final mesh generator.");

  MultiMooseEnum direction("x y z radial");
  params.addParam<MultiMooseEnum>("centroid_partitioner_directions",
                                  direction,
                                  "Specifies the sort direction if using the centroid partitioner. "
                                  "Available options: x, y, z, radial");

  params.addRequiredParam<VariableName>("temperature", "The coupled temperature variable.");
  params.addRequiredParam<std::vector<Real>>("emissivity", "Emissivities for each boundary.");
  return params;
}

RadiationTransferAction::RadiationTransferAction(const InputParameters & params)
  : Action(params),
    _boundary_ids(getParam<std::vector<boundary_id_type>>("sidesets")),
    _n_patches(getParam<std::vector<unsigned int>>("n_patches"))
{
}

void
RadiationTransferAction::act()
{
  if (_current_task == "add_mesh_generator")
    addMeshGenerator();
  else if (_current_task == "setup_mesh_complete")
    radiationPatchNames();
  else if (_current_task == "add_user_object")
  {
    addRadiationObject();
    addViewFactorObject();
  }
  else if (_current_task == "add_bc")
    addRadiationBCs();
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
  // add the view factor userobject; currently there is only one object implemented so no choices
  // in the future this section will allow switching different types
  InputParameters params = _factory.getValidParams("UnobstructedPlanarViewFactor");

  std::vector<std::vector<std::string>> radiation_patch_names = radiationPatchNames();
  std::vector<BoundaryName> boundary_names;
  for (auto & e1 : radiation_patch_names)
    for (auto & e2 : e1)
      boundary_names.push_back(e2);
  params.set<std::vector<BoundaryName>>("boundary") = boundary_names;

  // this userobject is only executed on initial
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL};
  params.set<ExecFlagEnum>("execute_on") = exec_enum;

  _problem->addUserObject("UnobstructedPlanarViewFactor", viewFactorObjectName(), params);
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
  if (emissivity.size() != _boundary_ids.size())
    mooseError("emissivity parameter needs to be the same size as the sidesets parameter.");

  // the action only sets up ViewFactorObjectSurfaceRadiation, because after splitting
  // faces auotmatically, it makes no sense to require view factor input by hand.
  InputParameters params = _factory.getValidParams("ViewFactorObjectSurfaceRadiation");
  params.set<std::vector<VariableName>>("temperature") = {getParam<VariableName>("temperature")};

  std::vector<Real> extended_emissivity;
  for (unsigned int j = 0; j < _boundary_ids.size(); ++j)
    for (unsigned int i = 0; i < _n_patches[j]; ++i)
      extended_emissivity.push_back(emissivity[j]);
  params.set<std::vector<Real>>("emissivity") = extended_emissivity;

  // add boundary parameter
  std::vector<BoundaryName> boundary_names;
  for (auto & e1 : radiation_patch_names)
    for (auto & e2 : e1)
      boundary_names.push_back(e2);
  params.set<std::vector<BoundaryName>>("boundary") = boundary_names;

  // add adiabatic_boundary parameter if required
  if (isParamValid("adiabatic_sidesets"))
  {
    std::vector<boundary_id_type> adiabatic_boundary_ids =
        getParam<std::vector<boundary_id_type>>("adiabatic_sidesets");
    std::vector<BoundaryName> adiabatic_boundary_names;
    for (unsigned int k = 0; k < adiabatic_boundary_ids.size(); ++k)
    {
      boundary_id_type abid = adiabatic_boundary_ids[k];

      // find the right entry in _boundary_ids
      auto it = std::find(_boundary_ids.begin(), _boundary_ids.end(), abid);

      // check if entry was found: it must be found or an error would occur later
      if (it == _boundary_ids.end())
        mooseError("Adiabatic sideset ", abid, " not present in sidesets.");

      // this is the position in the _boundary_ids vector; this is what
      // we are really after
      auto index = std::distance(_boundary_ids.begin(), it);

      // collect the correct boundary names
      for (auto & e : radiation_patch_names[index])
        adiabatic_boundary_names.push_back(e);
    }
    params.set<std::vector<BoundaryName>>("adiabatic_boundary") = adiabatic_boundary_names;
  }

  // add isothermal sidesets if required
  if (isParamValid("fixed_temperature_sidesets"))
  {
    if (!isParamValid("fixed_boundary_temperatures"))
      mooseError("fixed_temperature_sidesets is provided so fixed_boundary_temperatures must be "
                 "provided too");

    std::vector<boundary_id_type> fixed_T_boundary_ids =
        getParam<std::vector<boundary_id_type>>("fixed_temperature_sidesets");

    std::vector<FunctionName> fixed_T_funcs =
        getParam<std::vector<FunctionName>>("fixed_boundary_temperatures");

    // check length of fixed_boundary_temperatures
    if (fixed_T_funcs.size() != fixed_T_boundary_ids.size())
      mooseError("Size of parameter fixed_boundary_temperatures and fixed_temperature_sidesets "
                 "must be equal.");

    std::vector<BoundaryName> fixed_T_boundary_names;
    std::vector<FunctionName> fixed_T_function_names;
    for (unsigned int k = 0; k < fixed_T_boundary_ids.size(); ++k)
    {
      boundary_id_type bid = fixed_T_boundary_ids[k];

      // find the right entry in _boundary_ids
      auto it = std::find(_boundary_ids.begin(), _boundary_ids.end(), bid);

      // check if entry was found: it must be found or an error would occur later
      if (it == _boundary_ids.end())
        mooseError("Fixed temperature sideset ", bid, " not present in sidesets.");

      // this is the position in the _boundary_ids vector; this is what
      // we are really after
      auto index = std::distance(_boundary_ids.begin(), it);

      // collect the correct boundary names
      for (auto & e : radiation_patch_names[index])
      {
        fixed_T_boundary_names.push_back(e);
        fixed_T_function_names.push_back(fixed_T_funcs[k]);
      }
    }
    params.set<std::vector<BoundaryName>>("fixed_temperature_boundary") = fixed_T_boundary_names;
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
  std::vector<std::vector<std::string>> radiation_patch_names(_boundary_ids.size());
  for (unsigned int j = 0; j < _boundary_ids.size(); ++j)
  {
    boundary_id_type bid = _boundary_ids[j];
    std::string base_name = _mesh->getBoundaryName(bid);
    std::vector<std::string> bnames;
    for (unsigned int i = 0; i < _n_patches[j]; ++i)
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
  auto ad_bnd_ids = getParam<std::vector<boundary_id_type>>("adiabatic_sidesets");
  auto ft_bnd_ids = getParam<std::vector<boundary_id_type>>("fixed_temperature_sidesets");
  std::vector<std::vector<std::string>> radiation_patch_names;
  for (unsigned int j = 0; j < _boundary_ids.size(); ++j)
  {
    boundary_id_type bid = _boundary_ids[j];
    // check if this sideset is adiabatic or isothermal
    auto it_a = std::find(ad_bnd_ids.begin(), ad_bnd_ids.end(), bid);
    auto it_t = std::find(ft_bnd_ids.begin(), ft_bnd_ids.end(), bid);
    if (it_a != ad_bnd_ids.end() || it_t != ft_bnd_ids.end())
      continue;

    std::string base_name = _mesh->getBoundaryName(bid);
    std::vector<std::string> bnames;
    for (unsigned int i = 0; i < _n_patches[j]; ++i)
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
RadiationTransferAction::addMeshGenerator() const
{
  MultiMooseEnum partitioners = getParam<MultiMooseEnum>("partitioners");
  if (!_pars.isParamSetByUser("partitioners"))
  {
    partitioners.clear();
    for (unsigned int j = 0; j < _boundary_ids.size(); ++j)
      partitioners.push_back("metis");
  }

  MultiMooseEnum direction = getParam<MultiMooseEnum>("centroid_partitioner_directions");

  // check input parameters
  if (_boundary_ids.size() != _n_patches.size())
    mooseError("n_patches parameter must have same length as sidesets parameter.");

  if (_boundary_ids.size() != partitioners.size())
    mooseError("partitioners parameter must have same length as sidesets parameter.");

  for (unsigned int j = 0; j < partitioners.size(); ++j)
    if (partitioners[j] == "centroid" && direction.size() != _boundary_ids.size())
      mooseError(
          "centroid partitioner is selected for at least one sideset. "
          "centroid_partitioner_directions parameter must have same length as sidesets parameter.");

  // check if mesh is a MeshGeneratorMesh
  std::shared_ptr<MeshGeneratorMesh> mg_mesh = std::dynamic_pointer_cast<MeshGeneratorMesh>(_mesh);
  if (!mg_mesh)
    mooseError("This action adds MeshGenerator objects and therefore only works with a "
               "MeshGeneratorMesh.");

  MeshGeneratorName input = getParam<MeshGeneratorName>("final_mesh_generator");

  for (unsigned int j = 0; j < _boundary_ids.size(); ++j)
  {
    boundary_id_type bid = _boundary_ids[j];

    // create the name of this PatchSidesetGenerator
    std::stringstream ss;
    ss << "patch_side_set_generator_" << bid;
    MeshGeneratorName mg_name = ss.str();

    InputParameters params = _factory.getValidParams("PatchSidesetGenerator");
    params.set<MeshGeneratorName>("input") = input;
    params.set<boundary_id_type>("sideset") = bid;
    params.set<unsigned int>("n_patches") = _n_patches[j];
    params.set<MooseEnum>("partitioner") = partitioners[j];

    if (partitioners[j] == "centroid")
      params.set<MooseEnum>("centroid_partitioner_direction") = direction[j];

    _app.addMeshGenerator("PatchSidesetGenerator", mg_name, params);

    // reset input parameter to last one added
    input = mg_name;
  }
}
