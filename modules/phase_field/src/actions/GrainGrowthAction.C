//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainGrowthAction.h"

// MOOSE includes
#include "AddVariableAction.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseObjectAction.h"
#include "MooseMesh.h"
#include "NonlinearSystemBase.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("PhaseFieldApp", GrainGrowthAction, "add_aux_variable");
registerMooseAction("PhaseFieldApp", GrainGrowthAction, "add_aux_kernel");
registerMooseAction("PhaseFieldApp", GrainGrowthAction, "add_variable");
registerMooseAction("PhaseFieldApp", GrainGrowthAction, "add_kernel");
registerMooseAction("PhaseFieldApp", GrainGrowthAction, "copy_nodal_vars");
registerMooseAction("PhaseFieldApp", GrainGrowthAction, "check_copy_nodal_vars");

InputParameters
GrainGrowthAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set up the variable and the kernels needed for a grain growth simulation");
  params.addRequiredParam<unsigned int>("op_num",
                                        "specifies the number of order parameters to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<Real>(
      "scaling", 1.0, "Specifies a scaling factor to apply to the order parameters");
  params.addParam<bool>(
      "initial_from_file",
      false,
      "Take the initial condition of all polycrystal variables from the mesh file");

  // Get MooseEnums for the possible order/family options for this variable
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("family",
                             families,
                             "Specifies the family of FE "
                             "shape function to use for the order parameters");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE "
                             "shape function to use for the order parameters");

  params.addParam<MaterialPropertyName>(
      "mobility", "L", "The isotropic mobility used with the kernels");
  params.addParam<MaterialPropertyName>("kappa", "kappa_op", "The kappa used with the kernels");

  params.addParam<VariableName>("c", "Name of coupled concentration variable");

  params.addParam<Real>("en_ratio", 1.0, "Ratio of surface to GB energy");
  params.addParam<unsigned int>("ndef", 0, "Specifies the number of deformed grains to create");
  params.addParam<bool>("variable_mobility",
                        true,
                        "The mobility is a function of any MOOSE variable (if "
                        "this is set to false, L must be constant over the "
                        "entire domain!)");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments that L depends on");
  params.deprecateCoupledVar("args", "coupled_variables", "02/09/2024");

  params.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParam<bool>("use_automatic_differentiation",
                        false,
                        "Flag to use automatic differentiation (AD) objects when possible");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "Block restriction for the variables and kernels");

  params.addParamNamesToGroup("scaling implicit use_displaced_mesh", "Advanced");
  params.addParamNamesToGroup("c en_ratio ndef", "Multiphysics");

  return params;
}

GrainGrowthAction::GrainGrowthAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base")),
    _fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
    _use_ad(getParam<bool>("use_automatic_differentiation"))
{
}

void
GrainGrowthAction::act()
{
  // take initial values from file?
  bool initial_from_file = getParam<bool>("initial_from_file");

  // Loop over order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    auto type = AddVariableAction::variableType(_fe_type);
    auto var_params = _factory.getValidParams(type);

    var_params.applySpecificParameters(_pars, {"family", "order", "block"});
    var_params.set<std::vector<Real>>("scaling") = {getParam<Real>("scaling")};

    // Create variable name
    std::string var_name = _var_name_base + Moose::stringify(op);

    // Setup initial from file if requested
    if (initial_from_file)
    {
      if (_current_task == "check_copy_nodal_vars")
        _app.setExodusFileRestart(true);

      if (_current_task == "copy_nodal_vars")
      {
        auto * system = &_problem->getNonlinearSystemBase();
        system->addVariableToCopy(var_name, var_name, "LATEST");
      }
    }

    // Add variable
    if (_current_task == "add_variable")
      _problem->addVariable(type, var_name, var_params);

    // Add Kernels
    else if (_current_task == "add_kernel")
    {
      //
      // Add time derivative kernel
      //

      {
        std::string kernel_type = _use_ad ? "ADTimeDerivative" : "TimeDerivative";

        std::string kernel_name = var_name + "_" + kernel_type;
        InputParameters params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = var_name;
        params.applyParameters(parameters());

        addKernel(kernel_type, kernel_name, params);
      }

      //
      // Add ACGrGrPoly kernel
      //

      {
        std::string kernel_type = _use_ad ? "ADGrainGrowth" : "ACGrGrPoly";

        // Make vector of order parameter names, excluding this one
        std::vector<VariableName> v;
        v.resize(_op_num - 1);

        unsigned int ind = 0;
        for (unsigned int j = 0; j < _op_num; ++j)
          if (j != op)
            v[ind++] = _var_name_base + Moose::stringify(j);

        std::string kernel_name = var_name + "_" + kernel_type;
        InputParameters params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<std::vector<VariableName>>("v") = v;
        params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
        params.applyParameters(parameters());

        addKernel(kernel_type, kernel_name, params);
      }

      //
      // Add ACInterface kernel
      //

      {
        std::string kernel_type = _use_ad ? "ADACInterface" : "ACInterface";

        std::string kernel_name = var_name + "_" + kernel_type;
        InputParameters params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
        params.set<MaterialPropertyName>("kappa_name") = getParam<MaterialPropertyName>("kappa");
        params.set<bool>("variable_L") = getParam<bool>("variable_mobility");
        params.applyParameters(parameters());

        addKernel(kernel_type, kernel_name, params);
      }

      //
      // Set up optional ACGBPoly bubble interaction kernels
      //

      if (isParamValid("c"))
      {
        if (_use_ad)
          mooseError("AD version of ACGBPoly is not implemented");

        std::string kernel_type = "ACGBPoly";

        std::string kernel_name = var_name + "_" + kernel_type;
        InputParameters params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<std::vector<VariableName>>("c") = {getParam<VariableName>("c")};
        params.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params);
      }
    }
  }
  // Create auxvariable
  if (_current_task == "add_aux_variable")
  {
    auto var_params = _factory.getValidParams("MooseVariable");
    var_params.set<MooseEnum>("family") = "LAGRANGE";
    var_params.set<MooseEnum>("order") = "FIRST";
    var_params.applySpecificParameters(_pars, {"block"});
    _problem->addAuxVariable("MooseVariable", "bnds", var_params);
  }

  // Create BndsCalcAux auxkernel
  else if (_current_task == "add_aux_kernel")
  {
    // Make vector of order parameter names, excluding this one std::vector<VariableName> v;
    std::vector<VariableName> v;
    v.resize(_op_num);

    for (unsigned int j = 0; j < _op_num; ++j)
      v[j] = _var_name_base + Moose::stringify(j);

    std::string aux_kernel_type = "BndsCalcAux";

    std::string aux_kernel_name = "bnds_" + aux_kernel_type;
    InputParameters params = _factory.getValidParams(aux_kernel_type);
    params.set<AuxVariableName>("variable") = "bnds";
    params.set<std::vector<VariableName>>("v") = v;
    params.applyParameters(parameters());

    _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);
  }
}

void
GrainGrowthAction::addKernel(const std::string & kernel_type,
                             const std::string & kernel_name,
                             InputParameters params)
{
  _problem->addKernel(kernel_type, kernel_name, params);
}
