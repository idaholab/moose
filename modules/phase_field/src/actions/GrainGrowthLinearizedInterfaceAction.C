//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainGrowthLinearizedInterfaceAction.h"

// MOOSE includes
#include "GrainGrowthAction.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseObjectAction.h"
#include "MooseMesh.h"
#include "NonlinearSystemBase.h"

registerMooseAction("PhaseFieldApp", GrainGrowthLinearizedInterfaceAction, "add_aux_variable");
registerMooseAction("PhaseFieldApp", GrainGrowthLinearizedInterfaceAction, "add_aux_kernel");
registerMooseAction("PhaseFieldApp", GrainGrowthLinearizedInterfaceAction, "add_variable");
registerMooseAction("PhaseFieldApp", GrainGrowthLinearizedInterfaceAction, "add_kernel");
registerMooseAction("PhaseFieldApp", GrainGrowthLinearizedInterfaceAction, "add_material");
registerMooseAction("PhaseFieldApp", GrainGrowthLinearizedInterfaceAction, "add_bounds_vectors");
registerMooseAction("PhaseFieldApp", GrainGrowthLinearizedInterfaceAction, "copy_nodal_vars");
registerMooseAction("PhaseFieldApp", GrainGrowthLinearizedInterfaceAction, "check_copy_nodal_vars");

InputParameters
GrainGrowthLinearizedInterfaceAction::validParams()
{
  InputParameters params = GrainGrowthAction::validParams();
  params.addClassDescription("Set up the variable and the kernels needed for a grain growth "
                             "simulation with a linearized interface");
  params.addRequiredParam<std::string>("op_name_base",
                                       "specifies the base name of the dependent order parameters");
  params.addRequiredParam<Real>(
      "bound_value",
      "Bounds value used in the constrained solve, where limits are +/- bound_value");

  return params;
}

GrainGrowthLinearizedInterfaceAction::GrainGrowthLinearizedInterfaceAction(
    const InputParameters & params)
  : GrainGrowthAction(params), _op_name_base(getParam<std::string>("op_name_base"))
{
}

void
GrainGrowthLinearizedInterfaceAction::act()
{
  // Create Variables
  addVariables();

  // Loop over order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    // Create transformed variable name
    std::string var_name = _var_name_base + Moose::stringify(op);

    // Create dependent order parameter name
    std::string op_name = _op_name_base + Moose::stringify(op);

    // Add aux-variables
    if (_current_task == "add_aux_variable")
    {
      // Add aux-variable defining dependent order parameter
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("family") = "LAGRANGE";
      var_params.set<MooseEnum>("order") = "FIRST";
      _problem->addAuxVariable("MooseVariable", op_name, var_params);

      // Add bounds_dummy used for constrained solve
      var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("family") = "LAGRANGE";
      var_params.set<MooseEnum>("order") = "FIRST";
      _problem->addAuxVariable("MooseVariable", "bounds_dummy", var_params);
    }
    // Add the kernels for each grain growth variable
    else if (_current_task == "add_kernel")
    {
      //
      // Add time derivative kernel
      //

      {
        std::string kernel_type = "ChangedVariableTimeDerivative";

        std::string kernel_name = var_name + "_" + kernel_type;
        InputParameters params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<MaterialPropertyName>("order_parameter") = op_name;
        params.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params);
      }

      //
      // Add ACGrGrPolyLinearizedInterface kernel
      //

      {
        std::string kernel_type = "ACGrGrPolyLinearizedInterface";

        // Make vector of variable names and order parameter names, excluding this one
        std::vector<VariableName> v;
        v.resize(_op_num - 1);
        std::vector<MaterialPropertyName> other_ops;
        other_ops.resize(_op_num - 1);

        unsigned int ind = 0;
        for (unsigned int j = 0; j < _op_num; ++j)
          if (j != op)
          {
            v[ind] = _var_name_base + Moose::stringify(j);
            other_ops[ind] = _op_name_base + Moose::stringify(j);
            ind++;
          }

        std::string kernel_name = var_name + "_" + kernel_type;
        InputParameters params2 = _factory.getValidParams(kernel_type);
        params2.set<NonlinearVariableName>("variable") = var_name;
        params2.set<std::vector<VariableName>>("v") = v;
        params2.set<MaterialPropertyName>("this_op") = op_name;
        params2.set<std::vector<MaterialPropertyName>>("other_ops") = other_ops;
        params2.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
        params2.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params2);
      }

      //
      // Add ACInterface kernel
      //

      {
        std::string kernel_type = "ACInterfaceChangedVariable";

        std::string kernel_name = var_name + "_" + kernel_type;
        InputParameters params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<MaterialPropertyName>("mob_name") = getParam<MaterialPropertyName>("mobility");
        params.set<MaterialPropertyName>("kappa_name") = getParam<MaterialPropertyName>("kappa");
        params.set<MaterialPropertyName>("order_parameter") = op_name;
        params.set<bool>("variable_L") = getParam<bool>("variable_mobility");
        params.applyParameters(parameters());

        _problem->addKernel(kernel_type, kernel_name, params);
      }
    }

    // Add derivative-parsed material defining order parameter expressions
    if (_current_task == "add_material")
    {
      std::string material_name = "LinearizedInterfaceFunction";
      auto params = _factory.getValidParams(material_name);
      params.set<std::string>("property_name") = op_name;
      params.set<std::vector<VariableName>>("phi") = {var_name};

      _problem->addMaterial(material_name, op_name, params);
    }

    if (_current_task == "add_aux_kernel")
    {
      {
        // Add auxkernel for the order parameter auxvariable
        std::string aux_kernel_type = "LinearizedInterfaceAux";

        std::string aux_kernel_name = op_name + aux_kernel_type;
        InputParameters params = _factory.getValidParams(aux_kernel_type);
        params.set<AuxVariableName>("variable") = op_name;
        params.set<std::vector<VariableName>>("nonlinear_variable") = {var_name};
        params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
        params.applyParameters(parameters());

        _problem->addAuxKernel(aux_kernel_type, aux_kernel_name, params);
      }
      // Add upper and lower bound for each variable
      // Upper bound
      {
        std::string bound_type = "ConstantBounds";
        std::string bound_name = var_name + "_upper_bound";
        auto params = _factory.getValidParams(bound_type);
        params.set<AuxVariableName>("variable") = "bounds_dummy";
        params.set<NonlinearVariableName>("bounded_variable") = var_name;
        params.set<MooseEnum>("bound_type") = "upper";
        params.set<Real>("bound_value") = getParam<Real>("bound_value");
        _problem->addAuxKernel(bound_type, bound_name, params);
      }
      // Lower bound
      {
        std::string bound_type = "ConstantBounds";
        std::string bound_name = var_name + "_lower_bound";
        auto params = _factory.getValidParams(bound_type);
        params.set<AuxVariableName>("variable") = "bounds_dummy";
        params.set<NonlinearVariableName>("bounded_variable") = var_name;
        params.set<MooseEnum>("bound_type") = "lower";
        params.set<Real>("bound_value") = -1.0 * getParam<Real>("bound_value");
        _problem->addAuxKernel(bound_type, bound_name, params);
      }
    }
  }

  if (_current_task == "add_bounds_vectors")
  {
    _problem->getNonlinearSystemBase(/*nl_sys_num=*/0)
        .addVector("lower_bound", false, libMesh::GHOSTED);
    _problem->getNonlinearSystemBase(/*nl_sys_num=*/0)
        .addVector("upper_bound", false, libMesh::GHOSTED);
  }

  // Add AuxVriable and AuxKernel for Bnds variable
  addBnds(_op_name_base);
}
