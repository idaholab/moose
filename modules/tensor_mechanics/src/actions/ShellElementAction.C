//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseMesh.h"
#include "MooseObjectAction.h"
#include "ShellElementAction.h"
#include "CommonShellElementAction.h"

#include "libmesh/string_to_enum.h"
#include <algorithm>

registerMooseAction("TensorMechanicsApp", ShellElementAction, "create_problem");

registerMooseAction("TensorMechanicsApp", ShellElementAction, "add_variable");

registerMooseAction("TensorMechanicsApp", ShellElementAction, "add_kernel");

registerMooseAction("TensorMechanicsApp", ShellElementAction, "add_material");

InputParameters
ShellElementAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Sets up variables, stress divergence kernels and materials required "
                             "for a static analysis with shell elements.");
  params.addParam<bool>(
      "add_variables", false, "Add both displacement and rotation variables for shell elements.");
  params.addParam<std::vector<VariableName>>(
      "displacements", "The nonlinear displacement variables for the problem");
  params.addParam<std::vector<VariableName>>("rotations",
                                             "The nonlinear rotation variables for the problem");
  params.addRequiredCoupledVar(
      "thickness",
      "Thickness of the shell. Can be supplied as either a number or a variable name.");
  params.addRequiredParam<std::string>("through_thickness_order",
                                       "Quadrature order in out of plane direction");

  params.addParam<bool>(
      "large_strain", false, "Set to true to turn on finite strain calculations.");
  // MooseEnum strainType("INCREMENTAL FINITE", "INCREMENTAL");
  MooseEnum strainType("SMALL FINITE", "SMALL");
  params.addParam<MooseEnum>("strain_type", strainType, "Strain formulation");
  params.addParam<bool>("incremental", "Use incremental or total strain");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");

  params.addParam<std::string>("base_name", "Material property base name");

  // Advanced
  params.addParam<std::vector<AuxVariableName>>("save_in", "The displacement residuals");
  params.addParam<std::vector<AuxVariableName>>("diag_save_in",
                                                "The displacement diagonal preconditioner terms");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "The list of ids of the blocks (subdomain) that the "
                                              "stress divergence and materials will be applied to");

  // Planar Formulation
  MooseEnum planarFormulationType("NONE WEAK_PLANE_STRESS", "NONE");
  params.addParam<MooseEnum>(
      "planar_formulation", planarFormulationType, "Out-of-plane stress/strain formulation");
  params.addParam<VariableName>("out_of_plane_strain",
                                "Variable for the out-of-plane strain for plane stress models");

  return params;
}

ShellElementAction::ShellElementAction(const InputParameters & params)
  : Action(params),
    _large_strain(getParam<bool>("large_strain")),
    _incremental(getParam<bool>("incremental")),
    _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
    _subdomain_ids()
{
  _displacements = getParam<std::vector<VariableName>>("displacements");
  _ndisp = _displacements.size();
  _rotations = getParam<std::vector<VariableName>>("rotations");
  _nrot = _rotations.size();

  _t_qrule = std::make_unique<QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));

  _ntpoints = (_t_qrule->get_points()).size();

  if (!isParamValid("displacements"))
    paramError("displacements",
               "ShellElementAction: A vector of displacement variable names should be provided as "
               "input using `displacements`.");

  _strain_type = getParam<MooseEnum>("strain_type").getEnum<Strain>();

  // determine if displaced mesh is to be used
  _use_displaced_mesh = _strain_type != Strain::SMALL;
  if (params.isParamSetByUser("use_displaced_mesh"))
  {
    bool use_displaced_mesh_param = getParam<bool>("use_displaced_mesh");
    if (use_displaced_mesh_param != _use_displaced_mesh && params.isParamSetByUser("strain_type"))
      paramError("use_displaced_mesh",
                 "ShellElementAction: Wrong combination of "
                 "`use_displaced_mesh` and `strain_type`.");
    _use_displaced_mesh = use_displaced_mesh_param;
  }

  // Set values to variables after common parameters are applied
  _save_in = getParam<std::vector<AuxVariableName>>("save_in");
  _diag_save_in = getParam<std::vector<AuxVariableName>>("diag_save_in");

  if (_save_in.size() != 0 && _save_in.size() != _ndisp)
    paramError("save_in",
               "ShellElementAction: Number of save_in variables should equal to the number of "
               "displacement variables ",
               _ndisp);

  if (_diag_save_in.size() != 0 && _diag_save_in.size() != _ndisp)
    paramError("diag_save_in",
               "ShellElementAction: Number of diag_save_in variables should equal to the number of "
               "displacement variables ",
               _ndisp);
}

void
ShellElementAction::act()
{
  // Get the subdomain involved in the action once the mesh setup is complete
  if (_current_task == "create_problem")
  {
    // get subdomain IDs
    for (auto & name : _subdomain_names)
      _subdomain_ids.insert(_mesh->getSubdomainID(name));
  }

  if (_current_task == "add_variable")
  {
    //
    // Gather info from all other ShellElementAction
    //
    actGatherActionParameters();

    //
    // Add variables (optional)
    //
    actAddVariables();
  }

  //
  // Add Materials - ADComputeIncrementalShellStrain3D or ADComputeFiniteShellStrain
  // for shell elements
  //
  if (_current_task == "add_material")
    actAddMaterials();

  //
  // Add Kernels - ADStressDivergenceShell3D for shell
  //
  if (_current_task == "add_kernel")
    actAddKernels();
}

void
ShellElementAction::actGatherActionParameters()
{
  // Gather info about all other master actions when we add variables
  //
  if (getParam<bool>("add_variables"))
  {
    auto actions = _awh.getActions<ShellElementAction>();
    for (const auto & action : actions)
    {
      const auto size_before = _subdomain_id_union.size();
      const auto added_size = action->_subdomain_ids.size();
      _subdomain_id_union.insert(action->_subdomain_ids.begin(), action->_subdomain_ids.end());
      const auto size_after = _subdomain_id_union.size();

      if (size_after != size_before + added_size)
        paramError("block",
                   "ShellElementAction: The block restrictions in the LineElement actions must be "
                   "non-overlapping.");

      if (added_size == 0 && actions.size() > 1)
        paramError(
            "block",
            "ShellElementAction: No LineElement action can be block unrestricted if more than one "
            "LineElement action is specified.");
    }
  }
}

void
ShellElementAction::actAddVariables()
{
  if (getParam<bool>("add_variables"))
  {
    auto params = _factory.getValidParams("MooseVariable");

    // determine order of elements in mesh
    const bool second = _problem->mesh().hasSecondOrderElements();
    if (second)
      mooseError("ShellElementAction: Only linear elements are currently supported. "
                 "Please change the order of elements in the mesh to use first order elements.");

    params.set<MooseEnum>("order") = "FIRST";
    params.set<MooseEnum>("family") = "LAGRANGE";

    // Loop through the displacement variables
    for (const auto & disp : _displacements)
    {
      // Create displacement variables
      _problem->addVariable("MooseVariable", disp, params);
    }

    // Add rotation variables
    for (const auto & rot : _rotations)
    {
      // Create rotation variables
      _problem->addVariable("MooseVariable", rot, params);
    }
  }
}

void
ShellElementAction::actAddMaterials()
{
  // Add Strain
  if (_incremental)
  {
    auto params = _factory.getValidParams("ADComputeIncrementalShellStrain3D");
    params.applyParameters(parameters(), {});

    if (_strain_type == Strain::FINITE)
      params.set<bool>("large_strain") = true;

    _problem->addMaterial("ADComputeIncrementalShellStrain3D", name() + "_strain", params);
  }
  else
  {
    auto params = _factory.getValidParams("ADComputeFiniteShellStrain");
    params.applyParameters(parameters(), {});

    if (_strain_type == Strain::FINITE)
      params.set<bool>("large_strain") = true;

    _problem->addMaterial("ADComputeFiniteShellStrain", name() + "_strain", params);
  }
}

void
ShellElementAction::actAddKernels()
{
  auto params = _factory.getValidParams("ADStressDivergenceShell3D");
  params.applyParameters(parameters(), {"use_displaced_mesh", "save_in", "diag_save_in"});
  params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

  std::string kernel_name;
  for (unsigned int i = 0; i < (_ndisp + _nrot); ++i)
  {
    kernel_name = name() + "_ad_stress_divergence_shell_" + Moose::stringify(i);

    params.set<unsigned int>("component") = i;
    if (_save_in.size() == (_ndisp + _nrot))
      params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
    if (_diag_save_in.size() == (_ndisp + _nrot))
      params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};
    if (i < _ndisp)
      params.set<NonlinearVariableName>("variable") = _displacements[i];
    else
      params.set<NonlinearVariableName>("variable") = _rotations[i - 3];

    _problem->addKernel("ADStressDivergenceShell3D", kernel_name, params);
  }

  // for (unsigned int t = 0; t < _ntpoints; ++t)
  // {
  //   auto params = _factory.getValidParams("ADWeakPlaneStress");
  //   params.applyParameters(parameters(),
  //                          {"displacements",
  //                           "base_name",
  //                           "use_displaced_mesh",
  //                           "save_in",
  //                           "diag_save_in"
  //                           "out_of_plane_strain_direction"});
  //
  //   std::string wps_kernel_name = "t_points_" + Moose::stringify(t) + "_" + name();
  //
  //   params.set<std::string>("base_name") = "t_points_" + Moose::stringify(t);
  //   params.set<std::vector<VariableName>>("displacements") = _displacements;
  //   params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
  //   params.set<NonlinearVariableName>("variable") = getParam<VariableName>("out_of_plane_strain");
  //   for (unsigned int i = 0; i < (_ndisp + _nrot); ++i)
  //   {
  //     if (_save_in.size() == (_ndisp + _nrot))
  //       params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
  //     if (_diag_save_in.size() == (_ndisp + _nrot))
  //       params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};
  //   }
  //
  //   _problem->addKernel("ADWeakPlaneStress", wps_kernel_name, params);
  // }
}
