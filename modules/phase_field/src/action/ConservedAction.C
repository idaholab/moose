/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ConservedAction.h"
// MOOSE includes
#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseObjectAction.h"
#include "MooseMesh.h"
#include "AddVariableAction.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<ConservedAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription(
      "Set up the variable(s) and the kernels needed for a conserved phase field variable."
      " Note that for a direct solve, the element family and order are overwritten with hermite "
      "and third.");
  MooseEnum solves("DIRECT SPLIT");
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
  params.addParam<std::vector<VariableName>>("args",
                                             "Vector of variable arguments this kernel depends on");
  params.addRequiredParam<MaterialPropertyName>(
      "free_energy", "Base name of the free energy function F defined in a free energy material");
  params.addRequiredParam<MaterialPropertyName>("kappa", "The kappa used with the kernel");

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
    case SolveType::SPLIT:
      _fe_type = FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                        Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));
      break;
    default:
      mooseError("Incorrect solve_type in ConservedAction");
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
    // Create conserved variable _var_name
    _problem->addVariable(_var_name, _fe_type, _scaling);

    // Create chemical potential variable for split form
    switch (_solve_type)
    {
      case SolveType::DIRECT:
        break;
      case SolveType::SPLIT:
      {
        std::string chempot_name = "chem_pot_" + _var_name;
        _problem->addVariable(chempot_name, _fe_type, _scaling);
      }
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
      {
        // Add time derivative kernel
        std::string kernel_type = "TimeDerivative";

        std::string kernel_name = _var_name + "_" + kernel_type;
        InputParameters params1 = _factory.getValidParams(kernel_type);
        params1.set<NonlinearVariableName>("variable") = _var_name;
        params1.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params1);

        // Add CahnHilliard kernel
        kernel_type = "CahnHilliard";

        kernel_name = _var_name + "_" + kernel_type;
        InputParameters params2 = _factory.getValidParams(kernel_type);
        params2.set<NonlinearVariableName>("variable") = _var_name;
        params2.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
        params2.set<MaterialPropertyName>("f_name") = getParam<MaterialPropertyName>("free_energy");
        params2.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params2);

        // Add ACInterface kernel
        kernel_type = "CHInterface";

        kernel_name = _var_name + "_" + kernel_type;
        InputParameters params3 = _factory.getValidParams(kernel_type);
        params3.set<NonlinearVariableName>("variable") = _var_name;
        params3.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
        params3.set<MaterialPropertyName>("kappa_name") = getParam<MaterialPropertyName>("kappa");
        params3.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params3);
        break;
      }
      case SolveType::SPLIT:
      {
        std::string chempot_name = "chem_pot_" + _var_name;

        // Add time derivative kernel
        std::string kernel_type = "CoupledTimeDerivative";

        std::string kernel_name = _var_name + "_" + kernel_type;
        InputParameters params1 = _factory.getValidParams(kernel_type);
        params1.set<NonlinearVariableName>("variable") = chempot_name;
        params1.set<std::vector<VariableName>>("v") = {_var_name};
        params1.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params1);

        // Add SplitCHWRes kernel
        kernel_type = "SplitCHWRes";

        kernel_name = _var_name + "_" + kernel_type;
        InputParameters params2 = _factory.getValidParams(kernel_type);
        params2.set<NonlinearVariableName>("variable") = chempot_name;
        params2.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
        params2.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params2);

        // Add SplitCHParsed kernel
        kernel_type = "SplitCHParsed";

        kernel_name = _var_name + "_" + kernel_type;
        InputParameters params3 = _factory.getValidParams(kernel_type);
        params3.set<NonlinearVariableName>("variable") = _var_name;
        params3.set<std::vector<VariableName>>("w") = {chempot_name};
        params3.set<MaterialPropertyName>("f_name") = getParam<MaterialPropertyName>("free_energy");
        params3.set<MaterialPropertyName>("kappa_name") = getParam<MaterialPropertyName>("kappa");
        params3.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params3);
      }
    }
  }
}
