//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservedAction.h"
// MOOSE includes
#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseObjectAction.h"
#include "MooseMesh.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"

using namespace libMesh;

registerMooseAction("PhaseFieldApp", ConservedAction, "add_variable");

registerMooseAction("PhaseFieldApp", ConservedAction, "add_kernel");

InputParameters
ConservedAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set up the variable(s) and the kernels needed for a conserved phase field variable."
      " Note that for a direct solve, the element family and order are overwritten with hermite "
      "and third.");
  MooseEnum solves("DIRECT REVERSE_SPLIT FORWARD_SPLIT");
  params.addRequiredParam<MooseEnum>("solve_type", solves, "Split or direct solve?");
  // Get MooseEnums for the possible order/family options for this variable
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("family",
                             families,
                             "Specifies the family of FE "
                             "shape functions to use for this variable");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE "
                             "shape function to use for this variable");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addParam<bool>("implicit", true, "Whether kernels are implicit or not");
  params.addParam<bool>(
      "use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");
  params.addParamNamesToGroup("scaling implicit use_displaced_mesh", "Advanced");
  params.addRequiredParam<MaterialPropertyName>("mobility", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this kernel depends on");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");

  params.addRequiredParam<MaterialPropertyName>(
      "free_energy", "Base name of the free energy function F defined in a free energy material");
  params.addRequiredParam<MaterialPropertyName>("kappa", "The kappa used with the kernel");
  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "Block restriction for the variables and kernels");
  return params;
}

ConservedAction::ConservedAction(const InputParameters & params)
  : Action(params),
    _solve_type(getParam<MooseEnum>("solve_type").getEnum<SolveType>()),
    _var_name(name()),
    _scaling(getParam<Real>("scaling"))
{
  switch (_solve_type)
  {
    case SolveType::DIRECT:
      _fe_type = FEType(Utility::string_to_enum<Order>("THIRD"),
                        Utility::string_to_enum<FEFamily>("HERMITE"));
      if (!parameters().isParamSetByAddParam("order") &&
          !parameters().isParamSetByAddParam("family"))
        mooseWarning("Order and family autoset to third and hermite in ConservedAction");
      break;
    case SolveType::REVERSE_SPLIT:
    case SolveType::FORWARD_SPLIT:
      _fe_type = FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                        Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));
      // Set name of chemical potential variable
      _chempot_name = "chem_pot_" + _var_name;
      break;
    default:
      paramError("solve_type", "Incorrect solve_type in ConservedAction");
  }
}

void
ConservedAction::act()
{
  //
  // Add variable(s)
  //
  if (_current_task == "add_variable")
  {
    auto type = AddVariableAction::variableType(_fe_type);
    auto var_params = _factory.getValidParams(type);
    var_params.set<MooseEnum>("family") = Moose::stringify(_fe_type.family);
    var_params.set<MooseEnum>("order") = _fe_type.order.get_order();
    var_params.set<std::vector<Real>>("scaling") = {_scaling};
    var_params.applySpecificParameters(parameters(), {"block"});

    // Create conserved variable _var_name
    _problem->addVariable(type, _var_name, var_params);

    // Create chemical potential variable for split form
    switch (_solve_type)
    {
      case SolveType::DIRECT:
        break;
      case SolveType::REVERSE_SPLIT:
      case SolveType::FORWARD_SPLIT:
        _problem->addVariable(type, _chempot_name, var_params);
    }
  }

  //
  // Add Kernels
  //
  else if (_current_task == "add_kernel")
  {
    switch (_solve_type)
    {
      case SolveType::DIRECT:
        // Add time derivative kernel
        {
          std::string kernel_type = "TimeDerivative";

          std::string kernel_name = _var_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _var_name;
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }

        // Add CahnHilliard kernel
        {
          std::string kernel_type = "CahnHilliard";

          std::string kernel_name = _var_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _var_name;
          params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
          params.set<MaterialPropertyName>("f_name") =
              getParam<MaterialPropertyName>("free_energy");
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }

        // Add ACInterface kernel
        {
          std::string kernel_type = "CHInterface";

          std::string kernel_name = _var_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _var_name;
          params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
          params.set<MaterialPropertyName>("kappa_name") = getParam<MaterialPropertyName>("kappa");
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }
        break;

      case SolveType::REVERSE_SPLIT:
        // Add time derivative kernel
        {
          std::string kernel_type = "CoupledTimeDerivative";

          std::string kernel_name = _var_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _chempot_name;
          params.set<std::vector<VariableName>>("v") = {_var_name};
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }

        // Add SplitCHWRes kernel
        {
          std::string kernel_type = "SplitCHWRes";

          std::string kernel_name = _var_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _chempot_name;
          params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }

        // Add SplitCHParsed kernel
        {
          std::string kernel_type = "SplitCHParsed";

          std::string kernel_name = _var_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _var_name;
          params.set<std::vector<VariableName>>("w") = {_chempot_name};
          params.set<MaterialPropertyName>("f_name") =
              getParam<MaterialPropertyName>("free_energy");
          params.set<MaterialPropertyName>("kappa_name") = getParam<MaterialPropertyName>("kappa");
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }
        break;

      case SolveType::FORWARD_SPLIT:
        // Add time derivative kernel
        {
          std::string kernel_type = "TimeDerivative";

          std::string kernel_name = _var_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _var_name;
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }

        // Add MatDiffusion kernel for c residual
        {
          std::string kernel_type = "MatDiffusion";

          std::string kernel_name = _var_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _var_name;
          params.set<std::vector<VariableName>>("v") = {_chempot_name};
          params.set<MaterialPropertyName>("diffusivity") =
              getParam<MaterialPropertyName>("mobility");
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }
        // Add MatDiffusion kernel for chemical potential residual
        {
          std::string kernel_type = "MatDiffusion";

          std::string kernel_name = _chempot_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _chempot_name;
          params.set<std::vector<VariableName>>("v") = {_var_name};
          params.set<MaterialPropertyName>("diffusivity") = getParam<MaterialPropertyName>("kappa");
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }

        // Add CoupledMaterialDerivative kernel
        {
          std::string kernel_type = "CoupledMaterialDerivative";

          std::string kernel_name = _chempot_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _chempot_name;
          params.set<std::vector<VariableName>>("v") = {_var_name};
          params.set<MaterialPropertyName>("f_name") =
              getParam<MaterialPropertyName>("free_energy");
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }

        // Add CoefReaction kernel
        {
          std::string kernel_type = "CoefReaction";

          std::string kernel_name = _chempot_name + "_" + kernel_type;
          InputParameters params = _factory.getValidParams(kernel_type);
          params.set<NonlinearVariableName>("variable") = _chempot_name;
          params.set<Real>("coefficient") = -1.0;
          params.applyParameters(parameters());

          _problem->addKernel(kernel_type, kernel_name, params);
        }
    }
  }
}
