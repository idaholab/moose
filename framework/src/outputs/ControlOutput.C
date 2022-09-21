//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "ControlOutput.h"
#include "MooseApp.h"
#include "MooseObjectParameterName.h"
#include "InputParameterWarehouse.h"
#include "ConsoleUtils.h"

registerMooseObject("MooseApp", ControlOutput);

InputParameters
ControlOutput::validParams()
{
  // Get the base class parameters
  InputParameters params = Output::validParams();
  params.addClassDescription(
      "Output for displaying objects and parameters associated with the Control system.");

  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.addParam<bool>(
      "clear_after_output", true, "Clear the active control display after each output.");
  params.addParam<bool>("show_active_objects", true, "List active MooseObjects.");

  // Return the InputParameters
  return params;
}

ControlOutput::ControlOutput(const InputParameters & parameters)
  : Output(parameters),
    _clear_after_output(getParam<bool>("clear_after_output")),
    _show_active_objects(getParam<bool>("show_active_objects"))
{
}

void
ControlOutput::output(const ExecFlagType & type)
{
  if (type == EXEC_INITIAL)
    outputControls();
  else
    outputChangedControls();

  if (_show_active_objects)
    outputActiveObjects();
}

void
ControlOutput::outputActiveObjects()
{
  // Extract InputParameter objects from warehouse
  InputParameterWarehouse & wh = _app.getInputParameterWarehouse();
  const auto & params = wh.getInputParameters();

  // Populate a map based on unique InputParameter objects
  std::map<std::shared_ptr<InputParameters>, std::set<MooseObjectName>> objects;
  for (const auto & iter : params)
    objects[iter.second].insert(iter.first);

  // The stream to build
  std::stringstream oss;
  oss << std::left;

  // Loop through unique objects
  oss << "Active Objects:\n" << COLOR_DEFAULT;
  for (const auto & iter : objects)
  {
    std::shared_ptr<InputParameters> ptr = iter.first;
    // actions do not have 'enable' parameter
    if (!ptr->have_parameter<bool>("enable") || ptr->get<bool>("enable"))
    {
      // We print slightly differently in the first iteration of the loop.
      bool first_iteration = true;
      for (const auto & obj_name : iter.second)
      {
        if (first_iteration)
        {
          oss << ConsoleUtils::indent(2) << COLOR_YELLOW << obj_name << COLOR_DEFAULT << '\n';
          first_iteration = false;
        }
        else
          oss << ConsoleUtils::indent(4) << obj_name << '\n';
      }
    }
  }

  _console << oss.str() << std::endl;
}

void
ControlOutput::outputControls()
{
  InputParameterWarehouse & wh = _app.getInputParameterWarehouse();
  const auto & params = wh.getInputParameters();

  std::stringstream oss;
  oss << std::left;

  // Populate a map based on unique InputParameter objects
  std::map<std::shared_ptr<InputParameters>, std::set<MooseObjectName>> objects;
  for (const auto & iter : params)
    objects[iter.second].insert(iter.first);

  // Produce the control information
  oss << "Controls:\n";
  for (const auto & iter : objects)
  {
    std::shared_ptr<InputParameters> ptr = iter.first;

    const std::set<std::string> & names = ptr->getControllableParameters();

    if (!names.empty())
    {
      oss << ConsoleUtils::indent(2) << COLOR_YELLOW << ptr->get<std::string>("_object_name")
          << COLOR_DEFAULT << '\n';

      // Full names(s)
      oss << ConsoleUtils::indent(4) << "Name(s): ";
      for (const auto & obj_name : iter.second)
        oss << obj_name << " ";
      oss << '\n';

      // Tag(s)
      const std::vector<std::string> & tags = ptr->get<std::vector<std::string>>("control_tags");
      if (!tags.empty())
      {
        oss << ConsoleUtils::indent(4) << "Tag(s): ";
        for (const auto & tag_name : tags)
          oss << tag_name << " ";
        oss << '\n';
      }

      oss << ConsoleUtils::indent(4) << "Parameter(s):\n";
      for (const auto & param_name : names)
        oss << ConsoleUtils::indent(6) << std::setw(ConsoleUtils::console_field_width) << param_name
            << ptr->type(param_name) << '\n';
    }
  }

  _console << oss.str() << std::endl;
}

void
ControlOutput::outputChangedControls()
{
  InputParameterWarehouse & wh = _app.getInputParameterWarehouse();
  std::string dump = wh.dumpChangedControls(_clear_after_output);
  if (!dump.empty())
    _console << "\nActive Controls:\n" << dump << std::endl;
}
