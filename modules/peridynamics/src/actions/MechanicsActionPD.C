//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Factory.h"
#include "FEProblemBase.h"
#include "Conversion.h"
#include "MooseObjectAction.h"
#include "MechanicsActionPD.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("PeridynamicsApp", MechanicsActionPD, "setup_mesh_complete");
registerMooseAction("PeridynamicsApp", MechanicsActionPD, "create_problem_complete");
registerMooseAction("PeridynamicsApp", MechanicsActionPD, "add_aux_variable");
registerMooseAction("PeridynamicsApp", MechanicsActionPD, "add_user_object");
registerMooseAction("PeridynamicsApp", MechanicsActionPD, "add_ic");
registerMooseAction("PeridynamicsApp", MechanicsActionPD, "add_kernel");

InputParameters
MechanicsActionPD::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Class for setting up peridynamic kernels");

  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "Nonlinear variable names for the displacements");
  MooseEnum formulation_option("BOND ORDINARY_STATE NONORDINARY_STATE");
  params.addRequiredParam<MooseEnum>(
      "formulation", formulation_option, "Peridynamic formulation options");
  MooseEnum stabilization_option("FORCE BOND_HORIZON_I BOND_HORIZON_II");
  params.addParam<MooseEnum>("stabilization",
                             stabilization_option,
                             "Stabilization techniques for the peridynamic correspondence model");
  params.addParam<bool>(
      "full_jacobian",
      false,
      "Parameter to set whether to use full jacobian for state based formulation or not");
  MooseEnum strain_type("SMALL FINITE", "SMALL");
  params.addParam<MooseEnum>("strain", strain_type, "Strain formulation");
  params.addParam<VariableName>("temperature", "Nonlinear variable name for the temperature");
  params.addParam<VariableName>("out_of_plane_strain",
                                "Nonlinear variable name for the out_of_plane strain for "
                                "plane stress using the NOSPD formulation");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "List of ids of the blocks (subdomains) that the "
                                              "peridynamic mechanics kernel will be applied to");
  params.addParam<std::vector<AuxVariableName>>("save_in", "The displacement residuals");
  params.addParam<std::vector<AuxVariableName>>("diag_save_in",
                                                "The displacement diagonal preconditioner terms");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names",
      "List of eigenstrains to be coupled in non-ordinary state-based mechanics kernels");

  return params;
}

MechanicsActionPD::MechanicsActionPD(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _formulation(getParam<MooseEnum>("formulation")),
    _stabilization(getParam<MooseEnum>("stabilization")),
    _strain(getParam<MooseEnum>("strain")),
    _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
    _subdomain_ids(),
    _save_in(getParam<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in(getParam<std::vector<AuxVariableName>>("diag_save_in"))
{
  // Consistency check
  if (_formulation == "NONORDINARY_STATE" && !isParamValid("stabilization"))
    mooseError("'stabilization' is a required parameter for non-ordinary state-based models only.");

  if (_save_in.size() != 0 && _save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables ",
               _ndisp);

  if (_diag_save_in.size() != 0 && _diag_save_in.size() != _ndisp)
    mooseError(
        "Number of diag_save_in variables should equal to the number of displacement variables ",
        _ndisp);
}

void
MechanicsActionPD::act()
{
  if (_current_task == "setup_mesh_complete")
  {
    // get subdomain IDs
    for (auto & name : _subdomain_names)
      _subdomain_ids.insert(_mesh->getSubdomainID(name));
  }
  else if (_current_task == "create_problem_complete")
  {
    // Gather info about all other master actions for adding (aux)variables
    auto actions = _awh.getActions<MechanicsActionPD>();
    for (const auto & action : actions)
    {
      const auto size_before = _subdomain_id_union.size();
      const auto added_size = action->_subdomain_ids.size();
      _subdomain_id_union.insert(action->_subdomain_ids.begin(), action->_subdomain_ids.end());
      const auto size_after = _subdomain_id_union.size();

      if (size_after != size_before + added_size)
        mooseError("The block restrictions in the Peridynamics/Mechanics/Master actions must be "
                   "non-overlapping!");

      if (added_size == 0 && actions.size() > 1)
        mooseError(
            "No Peridynamics/Mechanics/Master action can be block unrestricted if more than one "
            "Peridynamics/Mechanics/Master action is specified!");
    }
  }
  else if (_current_task == "add_aux_variable")
  {
    // add the bond_status aux variable to all peridynamic domains
    const std::string var_type = "MooseVariableConstMonomial";
    InputParameters params = _factory.getValidParams(var_type);
    params.set<MooseEnum>("order") = "CONSTANT";
    params.set<MooseEnum>("family") = "MONOMIAL";

    if (!_subdomain_id_union.empty())
      for (const SubdomainID & id : _subdomain_id_union)
        params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));

    _problem->addAuxVariable(var_type, "bond_status", params);
  }
  else if (_current_task == "add_ic")
  {
    const std::string ic_type = "ConstantIC";
    const std::string ic_name = name() + "bond_status";

    InputParameters params = _factory.getValidParams(ic_type);
    params.set<VariableName>("variable") = "bond_status";
    params.set<Real>("value") = 1.0;

    // check whether this object is restricted to certain block?
    if (isParamValid("block"))
      params.set<std::vector<SubdomainName>>("block") = _subdomain_names;

    _problem->addInitialCondition(ic_type, ic_name, params);
  }
  else if (_current_task == "add_user_object")
  {
    // add ghosting UO
    const std::string uo_type = "GhostElemPD";
    const std::string uo_name = name() + "GhostElemPD";

    InputParameters params = _factory.getValidParams(uo_type);
    params.set<bool>("use_displaced_mesh") = (_strain == "FINITE");

    _problem->addUserObject(uo_type, uo_name, params);
  }
  else if (_current_task == "add_kernel")
  {
    const std::string kernel_name = getKernelName();
    InputParameters params = getKernelParameters(kernel_name);

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      const std::string kernel_object_name = name() + "Peridynamics_" + Moose::stringify(i);

      params.set<unsigned int>("component") = i;
      params.set<NonlinearVariableName>("variable") = _displacements[i];

      if (_save_in.size() != 0)
        params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};
      if (_diag_save_in.size() != 0)
        params.set<std::vector<AuxVariableName>>("diag_save_in") = {_diag_save_in[i]};

      _problem->addKernel(kernel_name, kernel_object_name, params);
    }
  }
  else
    mooseError("Task error in MechanicsActionPD!");
}

std::string
MechanicsActionPD::getKernelName()
{
  std::string name;

  if (_formulation == "BOND")
  {
    name = "MechanicsBPD";
  }
  else if (_formulation == "ORDINARY_STATE")
  {
    name = "MechanicsOSPD";
  }
  else if (_formulation == "NONORDINARY_STATE")
  {
    if (_stabilization == "FORCE")
    {
      name = "ForceStabilizedSmallStrainMechanicsNOSPD";
    }
    else if (_stabilization == "BOND_HORIZON_I")
    {
      if (_strain == "SMALL")
        name = "HorizonStabilizedFormISmallStrainMechanicsNOSPD";
      else
        name = "HorizonStabilizedFormIFiniteStrainMechanicsNOSPD";
    }
    else if (_stabilization == "BOND_HORIZON_II")
    {
      if (_strain == "SMALL")
        name = "HorizonStabilizedFormIISmallStrainMechanicsNOSPD";
      else
        name = "HorizonStabilizedFormIIFiniteStrainMechanicsNOSPD";
    }
    else
      paramError("stabilization", "Unknown PD stabilization scheme!");
  }
  else
    paramError("formulation", "Unsupported peridynamic formulation!");

  return name;
}

InputParameters
MechanicsActionPD::getKernelParameters(std::string name)
{
  InputParameters params = _factory.getValidParams(name);

  params.applyParameters(parameters(),
                         {"displacements",
                          "temperature",
                          "out_of_plane_strain",
                          "eigenstrain_names",
                          "use_displaced_mesh",
                          "save_in",
                          "diag_save_in"});

  params.set<std::vector<VariableName>>("displacements") = _displacements;
  if (isParamValid("temperature"))
    params.set<std::vector<VariableName>>("temperature") = {getParam<VariableName>("temperature")};
  if (isParamValid("out_of_plane_strain"))
    params.set<std::vector<VariableName>>("out_of_plane_strain") = {
        getParam<VariableName>("out_of_plane_strain")};

  // peridynamics modules are formulated based on initial configuration
  params.set<bool>("use_displaced_mesh") = false;

  if (_formulation == "NONORDINARY_STATE" && isParamValid("eigenstrain_names"))
  {
    params.set<std::vector<MaterialPropertyName>>("eigenstrain_names") =
        getParam<std::vector<MaterialPropertyName>>("eigenstrain_names");
  }

  return params;
}
