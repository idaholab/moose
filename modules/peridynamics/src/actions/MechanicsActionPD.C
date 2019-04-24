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

registerMooseAction("PeridynamicsApp", MechanicsActionPD, "add_aux_variable");
registerMooseAction("PeridynamicsApp", MechanicsActionPD, "add_kernel");
registerMooseAction("PeridynamicsApp", MechanicsActionPD, "add_user_object");
registerMooseAction("PeridynamicsApp", MechanicsActionPD, "setup_quadrature");

template <>
InputParameters
validParams<MechanicsActionPD>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Class for setting up peridynamic kernels");
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "Nonlinear variable names for the displacements");
  MooseEnum formulation_option("Bond OrdinaryState NonOrdinaryState", "Bond");
  params.addParam<MooseEnum>("formulation",
                             formulation_option,
                             "Available peridynamic formulation options: " +
                                 formulation_option.getRawNames());
  MooseEnum stabilization_option("Force Self", "Self");
  params.addParam<MooseEnum>(
      "stabilization",
      stabilization_option,
      "Available stabilization options for Non-Ordinary State Based Peridynamics: " +
          stabilization_option.getRawNames());
  params.addParam<bool>(
      "full_jacobian",
      false,
      "Parameter to set whether to use full jacobian for state based formulation or not");
  params.addParam<bool>("finite_strain_formulation",
                        false,
                        "Parameter to set whether the formulation is finite strain or not");
  params.addParam<VariableName>("temperature", "Nonlinear variable name for the temperature");
  params.addParam<VariableName>("out_of_plane_strain",
                                "Nonlinear variable name for the out_of_plane strain for "
                                "plane stress using SNOSPD formulation");
  params.addParam<bool>(
      "use_displaced_mesh",
      false,
      "Parameter to set whether to use the displaced mesh for computation or not");
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
    _finite_strain_formulation(getParam<bool>("finite_strain_formulation")),
    _save_in(getParam<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in(getParam<std::vector<AuxVariableName>>("diag_save_in"))
{
  // Consistency check
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
  if (_current_task == "add_aux_variable")
  {
    // add the bond_status aux variable which is not restricted to any specific block but for
    // all peridynamic domains
    _problem->addAuxVariable("bond_status",
                             FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                    Utility::string_to_enum<FEFamily>("MONOMIAL")));

    // Set the initial value to unit using InitialConditionAction, following the coding in
    // createInitialConditionAction() in AddVariableAction.C

    // Set the parameters for the action
    InputParameters action_params = _action_factory.getValidParams("AddOutputAction");
    action_params.set<ActionWarehouse *>("awh") = &_awh;
    action_params.set<std::string>("type") = "ConstantIC";
    // Create the action
    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("AddInitialConditionAction", "bond_status_moose", action_params));
    // Set the required parameters for the object to be created
    action->getObjectParams().set<VariableName>("variable") = "bond_status";
    action->getObjectParams().set<Real>("value") = 1.0;
    // Store the action in the ActionWarehouse
    _awh.addActionBlock(action);
  }
  else if (_current_task == "setup_quadrature") // set the quadrature type to GAUSS_LOBATTO and
                                                // order to FIRST, such that the two quadrature
                                                // points are the two end nodes of a Edge2 element
  {
    QuadratureType type = Moose::stringToEnum<QuadratureType>("GAUSS_LOBATTO");
    Order order = Moose::stringToEnum<Order>("FIRST");

    _problem->createQRules(type, order, order, order);
  }
  else if (_current_task == "add_user_object")
  {
    // add ghosting UO
    const std::string uo_type = "GhostElemPD";
    const std::string uo_name = "GhostElemPD";

    InputParameters params = _factory.getValidParams(uo_type);
    params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

    _problem->addUserObject(uo_type, uo_name, params);
  }
  else if (_current_task == "add_kernel")
  {
    const std::string kernel_name = getKernelName();
    InputParameters params = getKernelParameters(kernel_name);

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      const std::string kernel_object_name = "Peridynamics_" + Moose::stringify(i);

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
    mooseError("Task error in MechanicsActionPD");
}

std::string
MechanicsActionPD::getKernelName()
{
  std::string name;

  if (_formulation == "Bond")
    name = "MechanicsBPD";
  else if (_formulation == "OrdinaryState")
    name = "MechanicsOSPD";
  else if (_formulation == "NonOrdinaryState")
  {
    if (_stabilization == "Force")
      name = "ForceStabilizedSmallStrainMechanicsNOSPD";
    else if (_stabilization == "Self")
    {
      if (_finite_strain_formulation)
        name = "FiniteStrainMechanicsNOSPD";
      else
        name = "SmallStrainMechanicsNOSPD";
    }
    else
      paramError("stabilization", "Unknown PD stabilization scheme. Choose from: Force Self");
  }
  else
    paramError(
        "formulation",
        "Unsupported peridynamic formulation. Choose from: Bond OrdinaryState NonOrdinaryState");

  return name;
}

InputParameters
MechanicsActionPD::getKernelParameters(std::string name)
{
  InputParameters params = _factory.getValidParams(name);
  params.set<std::vector<VariableName>>("displacements") = _displacements;

  if (isParamValid("temperature"))
    params.set<VariableName>("temperature") = getParam<VariableName>("temperature");

  if (isParamValid("out_of_plane_strain"))
    params.set<VariableName>("out_of_plane_strain") = getParam<VariableName>("out_of_plane_strain");

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");

  params.set<bool>("full_jacobian") = getParam<bool>("full_jacobian");

  if (_formulation == "NonOrdinaryState")
  {
    params.set<std::vector<MaterialPropertyName>>("eigenstrain_names") =
        getParam<std::vector<MaterialPropertyName>>("eigenstrain_names");
  }

  // check whether this kernel is restricted to certain block?
  if (isParamValid("block"))
    params.set<std::vector<SubdomainName>>("block") = getParam<std::vector<SubdomainName>>("block");

  return params;
}
