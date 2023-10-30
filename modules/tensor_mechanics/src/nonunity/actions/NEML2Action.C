/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                       BlackBear                              */
/*                                                              */
/*           (c) 2017 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifdef NEML2_ENABLED

#include "neml2/misc/utils.h"
#include "neml2/misc/parser_utils.h"
#include "neml2/base/HITParser.h"
#include "NEML2Action.h"
#include "NEML2Utils.h"
#include "FEProblem.h"
#include "Factory.h"

registerMooseAction("BlackBearApp", NEML2Action, "parse_neml2");
registerMooseAction("BlackBearApp", NEML2Action, "add_material");
registerMooseAction("BlackBearApp", NEML2Action, "add_user_object");

InputParameters
NEML2Action::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Parse and set up NEML2 objects");
  params.addRequiredParam<FileName>("input",
                                    "Path to the NEML2 input file containing the NEML2 model(s)");
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
  params.addParam<MooseEnum>("mode", mode, "Mode of operation for the NEML2 material model.");
  mode.addDocumentation("ELEMENT", "Perform constitutive update element-by-element.");
  mode.addDocumentation("ALL", "Perform constitutive update for all quadrature points at once.");
  mode.addDocumentation("PARSE_ONLY",
                        "Only parse the NEML2 input file and manufacture the NEML2 model, do not "
                        "setup any MOOSE objects.");

  params.addParam<std::string>(
      "device",
      "cpu",
      "Device on which to evaluate the NEML2 model. The string supplied must follow the following "
      "schema: (cpu|cuda)[:<device-index>] where cpu or cuda specifies the device type, and "
      ":<device-index> optionally specifies a device index.");
  return params;
}

NEML2Action::NEML2Action(const InputParameters & params)
  : Action(params),
    _fname(getParam<FileName>("input")),
    _mname(getParam<std::string>("model")),
    _verbose(getParam<bool>("verbose")),
    _mode(getParam<MooseEnum>("mode")),
    _device(getParam<std::string>("device"))
{
}

void
NEML2Action::act()
{
  if (_current_task == "parse_neml2")
  {
    neml2::HITParser parser;
    const auto all_options = parser.parse(_fname);
    neml2::Factory::load(all_options);

    if (_verbose)
    {
      auto & model = neml2::Factory::get_object<neml2::Model>("Models", _mname);
      model.to(_device);

      _console << COLOR_YELLOW << "*** BEGIN NEML2 INFO***" << std::endl;
      _console << std::endl << "Device: " << _device << std::endl << std::endl;
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
      _problem->addUserObject(type, "_neml2_uo_" + _mode, params);
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
      _problem->addMaterial(type, "_neml2_stress_" + _mode, params);
    }
    else if (_mode == "ALL")
    {
      auto type = "CauchyStressFromNEML2Receiver";
      auto params = _factory.getValidParams(type);
      params.applyParameters(parameters());
      params.set<UserObjectName>("neml2_uo") = "_neml2_uo_" + _mode;
      _problem->addMaterial(type, "_neml2_stress_" + _mode, params);
    }
    else if (_mode == "PARSE_ONLY")
    {
      // no-op
    }
    else
      mooseError("Unsupported mode of constitutive update: ", _mode);
  }
}

#endif
