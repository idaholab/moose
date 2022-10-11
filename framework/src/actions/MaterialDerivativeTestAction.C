//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialDerivativeTestAction.h"

#include "Conversion.h"
#include "MooseEnum.h"
#include "FEProblemBase.h"
#include "MoosePreconditioner.h"
#include "NonlinearSystemBase.h"
#include "MooseVariableBase.h"

#include "libmesh/fe.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("MooseApp", MaterialDerivativeTestAction, "add_variable");

registerMooseAction("MooseApp", MaterialDerivativeTestAction, "add_kernel");

registerMooseAction("MooseApp", MaterialDerivativeTestAction, "add_preconditioning");

InputParameters
MaterialDerivativeTestAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Action for setting up the necessary objects for debugging material property derivatives.");
  params.addParam<std::vector<VariableName>>("args",
                                             "Variables the tested material property depends on.");
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "Name of the material property to test the derivatives of.");
  MooseEnum prop_type_enum("Real RankTwoTensor RankFourTensor");
  params.addParam<MooseEnum>(
      "prop_type", prop_type_enum, "Type of the material property to test the derivatives of.");
  params.addParam<unsigned int>(
      "derivative_order", 0, "Highest order derivative to test derivatives of.");
  return params;
}

MaterialDerivativeTestAction::MaterialDerivativeTestAction(const InputParameters & parameters)
  : Action(parameters),
    _args(getParam<std::vector<VariableName>>("args")),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _prop_type(getParam<MooseEnum>("prop_type").getEnum<PropTypeEnum>()),
    _derivative_order(getParam<unsigned int>("derivative_order")),
    _second(false),
    _derivatives({{_prop_name, {}}})
{
  std::vector<std::vector<std::vector<SymbolName>>> derivative_table(_derivative_order + 1);

  // 0th derivative is a (single) derivative w.r.t. to _no_ variables
  derivative_table[0] = {{}};

  // build higher order derivatives
  for (unsigned int n = 1; n <= _derivative_order; ++n)
    for (const auto & function : derivative_table[n - 1])
      for (const auto & var : _args)
      {
        // take previous order derivative and derive w.r.t. one of the args
        auto derivative = std::vector<SymbolName>(function);
        derivative.push_back(var);

        // add derivative to list
        derivative_table[n].push_back(derivative);
        _derivatives.insert(
            std::make_pair(derivativePropertyName(_prop_name, derivative), derivative));
      }
}

void
MaterialDerivativeTestAction::act()
{
  // finite element type
  const std::string order = _second ? "SECOND" : "FIRST";
  const std::string family("LAGRANGE");
  const auto type = "MooseVariable";
  auto params = _factory.getValidParams(type);
  params.set<MooseEnum>("order") = order;
  params.set<MooseEnum>("family") = family;

  // build higher order derivatives
  for (const auto & derivative : _derivatives)
  {
    // Create variables
    if (_current_task == "add_variable")
    {
      switch (_prop_type)
      {
        case PropTypeEnum::REAL:
          _problem->addVariable(type, "var_" + derivative.first, params);
          break;

        case PropTypeEnum::RANKTWOTENSOR:
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int j = 0; j < 3; ++j)
              _problem->addVariable(type,
                                    "var_" + derivative.first + '_' + Moose::stringify(i) + '_' +
                                        Moose::stringify(j),
                                    params);
          break;

        case PropTypeEnum::RANKFOURTENSOR:
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int j = 0; j < 3; ++j)
              for (unsigned int k = 0; k < 3; ++k)
                for (unsigned int l = 0; l < 3; ++l)
                  _problem->addVariable(type,
                                        "var_" + derivative.first + '_' + Moose::stringify(i) +
                                            '_' + Moose::stringify(j) + '_' + Moose::stringify(k) +
                                            '_' + Moose::stringify(l),
                                        params);
          break;

        default:
          mooseError("Unknown property type.");
      }
    }

    if (_current_task == "add_kernel")
    {
      switch (_prop_type)
      {
        case PropTypeEnum::REAL:
        {
          auto params = _factory.getValidParams("MaterialDerivativeTestKernel");
          params.set<std::vector<VariableName>>("args") = _args;
          params.set<std::vector<SymbolName>>("derivative") = derivative.second;
          params.set<MaterialPropertyName>("material_property") = _prop_name;
          params.set<NonlinearVariableName>("variable") = "var_" + derivative.first;
          _problem->addKernel("MaterialDerivativeTestKernel", "kernel_" + derivative.first, params);
          break;
        }

        case PropTypeEnum::RANKTWOTENSOR:
        {
          auto params = _factory.getValidParams("MaterialDerivativeRankTwoTestKernel");
          params.set<std::vector<VariableName>>("args") = _args;
          params.set<std::vector<SymbolName>>("derivative") = derivative.second;
          params.set<MaterialPropertyName>("material_property") = _prop_name;
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int j = 0; j < 3; ++j)
            {
              auto suffix =
                  derivative.first + '_' + Moose::stringify(i) + '_' + Moose::stringify(j);
              params.set<NonlinearVariableName>("variable") = "var_" + suffix;
              params.set<unsigned int>("i") = i;
              params.set<unsigned int>("j") = j;
              _problem->addKernel(
                  "MaterialDerivativeRankTwoTestKernel", "kernel_" + suffix, params);
            }
          break;
        }

        case PropTypeEnum::RANKFOURTENSOR:
        {
          auto params = _factory.getValidParams("MaterialDerivativeRankFourTestKernel");
          params.set<std::vector<VariableName>>("args") = _args;
          params.set<std::vector<SymbolName>>("derivative") = derivative.second;
          params.set<MaterialPropertyName>("material_property") = _prop_name;
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int j = 0; j < 3; ++j)
              for (unsigned int k = 0; k < 3; ++k)
                for (unsigned int l = 0; l < 3; ++l)
                {
                  auto suffix = derivative.first + '_' + Moose::stringify(i) + '_' +
                                Moose::stringify(j) + '_' + Moose::stringify(k) + '_' +
                                Moose::stringify(l);
                  params.set<NonlinearVariableName>("variable") = "var_" + suffix;
                  params.set<unsigned int>("i") = i;
                  params.set<unsigned int>("j") = j;
                  params.set<unsigned int>("k") = k;
                  params.set<unsigned int>("l") = l;
                  _problem->addKernel(
                      "MaterialDerivativeRankFourTestKernel", "kernel_" + suffix, params);
                }
          break;
        }

        default:
          mooseError("Unknown property type.");
      }
    }
  }

  if (_current_task == "add_preconditioning")
  {
    auto params = _factory.getValidParams("SMP");
    auto & row = params.set<std::vector<NonlinearVariableName>>("off_diag_row");
    auto & col = params.set<std::vector<NonlinearVariableName>>("off_diag_column");

    for (const auto & derivative : _derivatives)
    {
      switch (_prop_type)
      {
        case PropTypeEnum::REAL:
          for (auto & arg : _args)
          {
            row.push_back("var_" + derivative.first);
            col.push_back(arg);
          }
          break;

        case PropTypeEnum::RANKTWOTENSOR:
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int j = 0; j < 3; ++j)
              for (auto & arg : _args)
              {
                row.push_back("var_" + derivative.first + '_' + Moose::stringify(i) + '_' +
                              Moose::stringify(j));
                col.push_back(arg);
              }
          break;

        case PropTypeEnum::RANKFOURTENSOR:
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int j = 0; j < 3; ++j)
              for (unsigned int k = 0; k < 3; ++k)
                for (unsigned int l = 0; l < 3; ++l)
                  for (auto & arg : _args)
                  {
                    row.push_back("var_" + derivative.first + '_' + Moose::stringify(i) + '_' +
                                  Moose::stringify(j) + '_' + Moose::stringify(k) + '_' +
                                  Moose::stringify(l));
                    col.push_back(arg);
                  }
          break;

        default:
          mooseError("Unknown property type.");
      }
    }

    if (_problem.get() != nullptr)
    {
      std::shared_ptr<MoosePreconditioner> pc =
          _factory.create<MoosePreconditioner>("SMP", "material_derivative_SMP", params);

      _problem->getNonlinearSystemBase().setPreconditioner(pc);
    }
    else
      mooseError("_problem.get() returned nullptr");
  }
}
