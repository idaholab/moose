//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2Action.h"
#include "FEProblem.h"
#include "Factory.h"
#include "NEML2Utils.h"

#ifdef NEML2_ENABLED
#include "neml2/misc/utils.h"
#include "neml2/misc/parser_utils.h"
#include "neml2/base/HITParser.h"
#endif

registerMooseAction("SolidMechanicsApp", NEML2Action, "parse_neml2");
registerMooseAction("SolidMechanicsApp", NEML2Action, "add_material");
registerMooseAction("SolidMechanicsApp", NEML2Action, "add_user_object");

InputParameters
NEML2Action::validParams()
{
  InputParameters params = Action::validParams();
  NEML2Utils::addClassDescription(params, "Parse and set up NEML2 objects");
  params.addRequiredParam<DataFileName>(
      "input", "Path to the NEML2 input file containing the NEML2 model(s)");
  params.addRequiredParam<std::string>(
      "model",
      "Name of the NEML2 model, i.e., the string inside the brackets [] in the NEML2 input file "
      "that corresponds to the model you want to use.");
  params.addParam<bool>("verbose",
                        true,
                        "Whether to print additional information about the NEML2 model at the "
                        "beginning of the simulation");
  params.addParam<std::vector<VariableName>>(
      "temperature", std::vector<VariableName>{"0"}, "Coupled temperature");

  MooseEnum mode("ELEMENT ALL PARSE_ONLY", "ELEMENT");
  mode.addDocumentation("ELEMENT", "Perform constitutive update element-by-element.");
  mode.addDocumentation("ALL", "Perform constitutive update for all quadrature points at once.");
  mode.addDocumentation("PARSE_ONLY",
                        "Only parse the NEML2 input file and manufacture the NEML2 model, do not "
                        "setup any MOOSE objects.");
  params.addParam<MooseEnum>("mode", mode, "Mode of operation for the NEML2 material model.");

  params.addParam<std::string>(
      "device",
      "cpu",
      "Device on which to evaluate the NEML2 model. The string supplied must follow the following "
      "schema: (cpu|cuda)[:<device-index>] where cpu or cuda specifies the device type, and "
      ":<device-index> optionally specifies a device index.");
  params.addParam<bool>("enable_AD",
                        false,
                        "Set to true to enable PyTorch AD. When set to false (default), no "
                        "function graph or gradient is computed, which speeds up model "
                        "evaluation.");
  return params;
}

#ifndef NEML2_ENABLED

NEML2Action::NEML2Action(const InputParameters & parameters) : Action(parameters)
{
  NEML2Utils::libraryNotEnabledError(parameters);
}

void
NEML2Action::act()
{
}

#else

NEML2Action::NEML2Action(const InputParameters & parameters)
  : Action(parameters),
    _fname(getDataFileName("input")),
    _mname(getParam<std::string>("model")),
    _verbose(getParam<bool>("verbose")),
    _mode(getParam<MooseEnum>("mode")),
    _device(getParam<std::string>("device")),
    _enable_AD(getParam<bool>("enable_AD"))
{
}

void
NEML2Action::act()
{
  const auto mode_name = std::string(_mode);
  if (_current_task == "parse_neml2")
  {
    neml2::load_input(std::string(_fname));

    if (_verbose)
    {
      auto & model = neml2::get_model(_mname, _enable_AD);

      _console << COLOR_YELLOW << "*** BEGIN NEML2 INFO ***" << std::endl;
      _console << model << std::endl;
      _console << "*** END NEML2 INFO ***" << COLOR_DEFAULT << std::endl;
    }
  }

  if (_current_task == "add_user_object")
  {
    if (_mode == "ELEMENT")
    {
      // no-op
    }
    else if (_mode == "ALL")
    {
      auto type = "CauchyStressFromNEML2UO";
      auto params = _factory.getValidParams(type);
      params.applyParameters(parameters());
      _problem->addUserObject(type, "_neml2_uo_" + mode_name, params);
    }
    else if (_mode == "PARSE_ONLY")
    {
      // no-op
    }
    else
      mooseError("Unsupported mode of constitutive update: ", _mode);
  }

  if (_current_task == "add_material")
  {
    if (_mode == "ELEMENT")
    {
      auto type = "CauchyStressFromNEML2";
      auto params = _factory.getValidParams(type);
      params.applyParameters(parameters());
      _problem->addMaterial(type, "_neml2_stress_" + mode_name, params);
    }
    else if (_mode == "ALL")
    {
      auto type = "CauchyStressFromNEML2Receiver";
      auto params = _factory.getValidParams(type);
      params.applyParameters(parameters());
      params.set<UserObjectName>("neml2_uo") = "_neml2_uo_" + mode_name;
      _problem->addMaterial(type, "_neml2_stress_" + mode_name, params);
    }
    else if (_mode == "PARSE_ONLY")
    {
      // no-op
    }
    else
      mooseError("Unsupported mode of constitutive update: ", _mode);
  }
}

#endif // NEML2_ENABLED
